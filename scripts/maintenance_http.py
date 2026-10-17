"""Bounded HTTPS GETs for maintenance: explicit hosts, pinned DNS and manual redirects.

No proxies, cookies, credentials, automatic approvals or third-party dependencies.
"""
from contextlib import contextmanager
from dataclasses import dataclass
import http.client
import ipaddress
import json
from pathlib import Path
import queue
import re
import socket
import ssl
import threading
import time
import urllib.error
from urllib.parse import quote, urljoin, urlsplit, urlunsplit

from scientific_sources.access import access_issue

POLICY = Path(__file__).with_name('maintenance_http_policy.json')
REDIRECTS = {301, 302, 303, 307, 308}


class UnsafeDestination(ValueError):
    """A destination is outside maintenance policy; not a scientific decision."""


def clean_url_text(value):
    if not isinstance(value, str) or not value or len(value) > 8192 or '\\' in value or any(ord(c) <= 32 or ord(c) == 127 for c in value):
        raise UnsafeDestination('Invalid URL text')


@dataclass(frozen=True)
class Target:
    url: str
    host: str
    path: str


class DestinationPolicy:
    def __init__(self):
        data = json.loads(POLICY.read_text(encoding='utf-8'))
        if data.get('schema_version') != 1 or data.get('allowed_ports') != [443]:
            raise UnsafeDestination('Invalid maintenance policy')
        if not isinstance(data.get('allowed_hosts'), list) or not data['allowed_hosts'] or any(
                not isinstance(h, str) or not re.fullmatch(r'[a-z0-9]+(?:[.-][a-z0-9]+)*', h) for h in data['allowed_hosts']):
            raise UnsafeDestination('Invalid explicit host list')
        self.hosts = frozenset(data['allowed_hosts'])

    def validate(self, url):
        clean_url_text(url)
        try:
            parts = urlsplit(url)
            host = (parts.hostname or '').lower()
            port = parts.port
        except ValueError:
            raise UnsafeDestination('Malformed destination') from None
        restriction = access_issue(url)
        if restriction:
            raise PermissionError(restriction)
        if parts.scheme != 'https' or port not in (None, 443) or parts.username is not None or parts.password is not None:
            raise UnsafeDestination('Only credential-free HTTPS on port 443 is permitted')
        if not re.fullmatch(r'[a-z0-9]+(?:[.-][a-z0-9]+)*', host) or host not in self.hosts:
            raise UnsafeDestination('Host not in explicit maintenance policy')
        # IP literals and alternative numeric forms are never policy hosts.
        if re.fullmatch(r'(?:0x[0-9a-f]+|[0-9]+)(?:\.(?:0x[0-9a-f]+|[0-9]+)){0,3}', host):
            raise UnsafeDestination('IP literal destinations are not permitted')
        path = quote(parts.path or '/', safe="/%:@!$&'()*+,;=-._~")
        query_text = quote(parts.query, safe="/%?:@!$&'()*+,;=-._~")
        canonical = urlunsplit(('https', host, path, query_text, ''))
        return Target(canonical, host, path + ('?' + query_text if query_text else ''))


def public_address(text):
    try:
        address = ipaddress.ip_address(text)
    except ValueError:
        return False
    if not address.is_global or address.is_reserved or address.is_multicast or address.is_loopback or address.is_link_local or address.is_unspecified:
        return False
    if address.version == 6:
        # No scoped, mapped, transition or translation addresses. Native public
        # global-unicast IPv6 is sufficient for the documented web destinations.
        return address in ipaddress.ip_network('2000::/3') and address not in ipaddress.ip_network('2002::/16') and address not in ipaddress.ip_network('2001::/32')
    return address not in ipaddress.ip_network('192.0.0.0/24') and address not in ipaddress.ip_network('192.88.99.0/24')


@dataclass(frozen=True)
class Endpoint:
    family: int
    address: str

    @property
    def sockaddr(self):
        return (self.address, 443) if self.family == socket.AF_INET else (self.address, 443, 0, 0)


class PublicResolver:
    def __init__(self, resolver=None):
        self.resolver = resolver or socket.getaddrinfo
        self.lock = threading.Lock()
        self.busy = False

    def resolve(self, host, timeout):
        # Native getaddrinfo has no portable timeout. One daemon worker per
        # client bounds waiting without accumulating workers after a timeout.
        if timeout <= 0:
            raise TimeoutError('DNS resolution exceeded budget')
        with self.lock:
            if self.busy:
                raise TimeoutError('Previous DNS resolution still pending')
            self.busy = True
        result = queue.Queue(maxsize=1)

        def lookup():
            try:
                value = self.resolver(host, 443, socket.AF_UNSPEC, socket.SOCK_STREAM, socket.IPPROTO_TCP)
                outcome = (value, None)
            except Exception as error:
                outcome = (None, error)
            with self.lock:
                self.busy = False
            result.put(outcome)

        threading.Thread(target=lookup, daemon=True).start()
        try:
            rows, error = result.get(timeout=timeout)
        except queue.Empty:
            raise TimeoutError('DNS resolution exceeded budget') from None
        if error:
            raise OSError('DNS resolution failed') from error
        if not rows or len(rows) > 64:
            raise UnsafeDestination('Empty or excessive DNS answer')
        endpoints = []
        for family, kind, protocol, _, sockaddr in rows:
            text = sockaddr[0]
            if (family not in (socket.AF_INET, socket.AF_INET6) or kind != socket.SOCK_STREAM
                    or protocol not in (0, socket.IPPROTO_TCP) or sockaddr[1] != 443 or '%' in text
                    or (family == socket.AF_INET6 and (len(sockaddr) != 4 or sockaddr[3] != 0))
                    or not public_address(text)):
                raise UnsafeDestination('DNS contains a forbidden endpoint')
            address = ipaddress.ip_address(text)
            if (address.version == 4) != (family == socket.AF_INET):
                raise UnsafeDestination('DNS address family mismatch')
            endpoint = Endpoint(family, str(address))
            if endpoint not in endpoints:
                endpoints.append(endpoint)
        return endpoints


class PinnedHTTPSConnection(http.client.HTTPConnection):
    default_port = 443

    def __init__(self, host, endpoint, timeout, deadline):
        super().__init__(host, 443, timeout=timeout)
        self.endpoint = endpoint
        self.deadline = deadline
        self.bound_socket = None
        self.expired = False
        self.watchdog = None

    def abort(self):
        self.expired = True
        if self.bound_socket is not None:
            try:
                self.bound_socket.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass

    def connect(self):
        remaining = self.deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError('Maintenance request deadline exceeded')
        raw = socket.socket(self.endpoint.family, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        self.bound_socket = raw
        self.watchdog = threading.Timer(remaining, self.abort)
        self.watchdog.daemon = True
        self.watchdog.start()
        try:
            raw.settimeout(min(self.timeout, remaining))
            # Numeric sockaddr: no second hostname resolution between validation
            # and connection, and no environment proxy can replace the endpoint.
            raw.connect(self.endpoint.sockaddr)
            peer = raw.getpeername()
            if ipaddress.ip_address(peer[0]) != ipaddress.ip_address(self.endpoint.address) or peer[1] != 443:
                raise UnsafeDestination('Connected peer differs from validated endpoint')
            context = ssl.create_default_context()
            if not context.check_hostname or context.verify_mode != ssl.CERT_REQUIRED:
                raise UnsafeDestination('TLS certificate verification is required')
            wrapped = context.wrap_socket(raw, server_hostname=self.host, do_handshake_on_connect=False)
            self.bound_socket = wrapped
            if self.expired:
                raise TimeoutError('Maintenance request deadline exceeded')
            wrapped.do_handshake()
            self.sock = wrapped
        except BaseException:
            self.finish()
            raise

    def finish(self):
        if self.watchdog is not None:
            self.watchdog.cancel()
        self.close()
        if self.bound_socket is not None:
            self.bound_socket.close()


class SocketTransport:
    @contextmanager
    def open(self, target, endpoints, timeout, deadline):
        connection = None
        for endpoint in endpoints[:4]:
            connection = PinnedHTTPSConnection(target.host, endpoint, timeout, deadline)
            try:
                connection.connect()
                break
            except (OSError, TimeoutError):
                connection.finish()
                if endpoint == endpoints[:4][-1]:
                    raise
        response = None
        try:
            connection.request('GET', target.path, headers={
                'User-Agent': 'ProjetoGlossata/1.0 evidence-maintenance',
                'Accept-Encoding': 'identity', 'Connection': 'close'})
            # Own the response lifecycle. HTTPConnection.getresponse() closes
            # its socket early for "Connection: close", which would undermine
            # the watchdog while the body still holds a file reference.
            response = http.client.HTTPResponse(connection.sock, method='GET')
            response.begin()
            yield response
            if connection.expired:
                raise TimeoutError('Maintenance request deadline exceeded')
        finally:
            if response is not None:
                response.close()
            connection.finish()


class PublicClient:
    def __init__(self, *, resolver=None, transport=None, sleep=None, timeout=15, total_timeout=45, max_redirects=5):
        if not 0 < timeout <= 15 or not 0 < total_timeout <= 45 or type(max_redirects) is not int or not 0 <= max_redirects <= 5:
            raise ValueError('Maintenance limits outside policy')
        self.policy = DestinationPolicy()
        self.resolver = PublicResolver(resolver)
        self.transport = transport or SocketTransport()
        self.sleep = sleep or time.sleep
        self.timeout = timeout
        self.total_timeout = total_timeout
        self.max_redirects = max_redirects
        self.last = None

    def get(self, url, as_json=False):
        deadline = time.monotonic() + self.total_timeout
        visited = set()
        for hop in range(self.max_redirects + 1):
            target = self.policy.validate(url)
            if target.url in visited:
                raise UnsafeDestination('Redirect loop')
            visited.add(target.url)
            delay = max(0, 1.1 - (time.monotonic() - self.last)) if self.last is not None else 0
            if time.monotonic() + delay >= deadline:
                raise TimeoutError('Maintenance request deadline exceeded')
            self.sleep(delay)
            endpoints = self.resolver.resolve(target.host, min(self.timeout, deadline - time.monotonic()))
            self.last = time.monotonic()
            with self.transport.open(target, endpoints, self.timeout, deadline) as response:
                if response.status in REDIRECTS:
                    locations = response.headers.get_all('Location', [])
                    if len(locations) != 1 or hop == self.max_redirects:
                        raise UnsafeDestination('Missing, ambiguous or excessive redirect')
                    location = locations[0].strip(' \t')  # HTTP field OWS only.
                    clean_url_text(location)
                    url = urljoin(target.url, location)
                    continue
                if 300 <= response.status < 400:
                    raise UnsafeDestination('Unsupported redirect status')
                if response.status >= 400:
                    raise urllib.error.HTTPError(target.url, response.status, response.reason, response.headers, None)
                if not 200 <= response.status < 300:
                    raise UnsafeDestination('Unexpected HTTP status')
                maximum = 4_000_000 if as_json else 1024
                body = bytearray()
                while len(body) < maximum + int(as_json):
                    if time.monotonic() >= deadline:
                        raise TimeoutError('Maintenance request deadline exceeded')
                    chunk = response.read1(min(65536, maximum + int(as_json) - len(body)))
                    if not chunk:
                        break
                    body.extend(chunk)
                if time.monotonic() >= deadline:
                    raise TimeoutError('Maintenance request deadline exceeded')
                if as_json and len(body) > maximum:
                    raise ValueError('Resposta excede o limite de manutenção.')
                return json.loads(body) if as_json else {'http_status': response.status, 'final_url': target.url}
        raise UnsafeDestination('Redirect limit exceeded')
