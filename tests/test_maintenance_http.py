"""Production HTTP parser over simulated sockets/TLS/DNS; no Internet requests."""
import io
import json
from pathlib import Path
import socket
import ssl
import sys
import threading
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
import maintenance_http as http
import review_updates


def answer(address='93.184.216.34'):
    family = socket.AF_INET6 if ':' in address else socket.AF_INET
    sockaddr = (address, 443, 0, 0) if family == socket.AF_INET6 else (address, 443)
    return (family, socket.SOCK_STREAM, socket.IPPROTO_TCP, '', sockaddr)


def response(code=200, location=None, body=b'ok', extra=''):
    if isinstance(body, str): body = body.encode()
    headers = f'HTTP/1.1 {code} Synthetic\r\nConnection: close\r\n'
    if location is not None: headers += 'Location: ' + location + '\r\n'
    return headers.encode() + extra.encode() + b'\r\n' + body


class Clock:
    now = 1000.0
    def monotonic(self): return self.now
    def sleep(self, seconds): self.now += seconds


class FakeTimer:
    def __init__(self, interval, callback):
        self.interval = interval; self.callback = callback; self.daemon = False; self.cancelled = False
    def start(self): pass
    def cancel(self): self.cancelled = True


class Socket:
    def __init__(self, network, family):
        self.network = network; self.family = family; self.host = network.last_host
        self.closed = False; self.sent = b''; self.shutdown_called = False
    def settimeout(self, timeout): self.timeout = timeout
    def connect(self, address):
        self.address = address; self.network.connected.append((self.host, address))
    def getpeername(self): return self.network.peer or self.address
    def do_handshake(self): self.network.handshakes.append(self.host)
    def sendall(self, data): self.sent += data
    def makefile(self, *args): return io.BytesIO(self.network.responses.pop(0))
    def shutdown(self, *args): self.shutdown_called = True
    def close(self): self.closed = True


class TLS:
    check_hostname = True
    verify_mode = ssl.CERT_REQUIRED
    def __init__(self, network): self.network = network
    def wrap_socket(self, raw, *, server_hostname, do_handshake_on_connect):
        self.network.sni.append(server_hostname)
        assert not do_handshake_on_connect
        assert raw.host == server_hostname
        return raw


class Network:
    def __init__(self, responses, addresses=None):
        self.responses = list(responses); self.addresses = addresses or {}
        self.resolved = []; self.connected = []; self.sockets = []; self.handshakes = []; self.sni = []
        self.peer = None; self.last_host = None
    def resolve(self, host, *args):
        self.last_host = host; self.resolved.append(host)
        addresses = self.addresses.get(host, ['93.184.216.34'])
        if callable(addresses): addresses = addresses()
        return [answer(a) for a in addresses]
    def socket(self, family, kind, protocol):
        raw = Socket(self, family); self.sockets.append(raw); return raw


class MaintenanceTests(unittest.TestCase):
    def setUp(self):
        self.clock = Clock(); self.timers = []
        self.enterContext(patch.object(http.time, 'monotonic', self.clock.monotonic))
        def timer(interval, callback):
            value = FakeTimer(interval, callback); self.timers.append(value); return value
        self.enterContext(patch.object(http.threading, 'Timer', side_effect=timer))
        self.enterContext(patch.object(socket, 'getaddrinfo', side_effect=AssertionError('Real DNS forbidden in tests')))

    def client(self, network, **kwargs):
        self.enterContext(patch.object(socket, 'socket', side_effect=network.socket))
        self.enterContext(patch.object(ssl, 'create_default_context', side_effect=lambda: TLS(network)))
        return http.PublicClient(resolver=network.resolve, sleep=self.clock.sleep, **kwargs)

    def test_compatibility_import_and_api_json_keep_original_host_tls_and_pinned_ip(self):
        self.assertIs(review_updates.PublicClient, http.PublicClient)
        network = Network([response(body=b'{"results":[]}')])
        result = self.client(network).get('https://api.inaturalist.org/v1/taxa/1', as_json=True)
        self.assertEqual(result, {'results': []})
        self.assertEqual(network.resolved, ['api.inaturalist.org'])
        self.assertEqual(network.connected, [('api.inaturalist.org', ('93.184.216.34', 443))])
        self.assertEqual(network.sni, ['api.inaturalist.org'])
        self.assertIn(b'Host: api.inaturalist.org\r\n', network.sockets[0].sent)
        self.assertTrue(all(s.closed for s in network.sockets))
        self.assertTrue(all(t.daemon and t.cancelled and t.interval <= 45 for t in self.timers))

    def test_legitimate_doi_cross_host_and_relative_redirect_chain(self):
        network = Network([response(302, 'https://www.nature.com/article'), response(307, '/article/latest'), response()])
        result = self.client(network).get('https://doi.org/10.1038/example')
        self.assertEqual(result['final_url'], 'https://www.nature.com/article/latest')
        self.assertEqual(network.resolved, ['doi.org', 'www.nature.com', 'www.nature.com'])
        self.assertGreaterEqual(self.clock.now, 1002.2)
        self.assertEqual(network.sni, network.resolved)

    def test_supported_redirect_statuses_and_exact_limit(self):
        for code in (301, 302, 303, 307, 308):
            with self.subTest(code=code):
                network = Network([response(code, '/final'), response()])
                self.assertEqual(self.client(network, max_redirects=1).get('https://doi.org/start')['http_status'], 200)
        network = Network([response(302, '/a'), response(302, '/b')])
        with self.assertRaises(http.UnsafeDestination): self.client(network, max_redirects=1).get('https://doi.org/start')
        self.assertEqual(len(network.connected), 2)

    def test_default_allows_five_redirects_and_refuses_the_sixth(self):
        network = Network([response(302, '/hop' + str(i)) for i in range(5)] + [response()])
        self.assertEqual(self.client(network).get('https://doi.org/start')['http_status'], 200)
        self.assertEqual(len(network.connected), 6)
        network = Network([response(302, '/hop' + str(i)) for i in range(6)])
        with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/start')
        self.assertEqual(len(network.connected), 6)

    def test_downgrade_protocols_credentials_ports_and_local_addresses_blocked_before_dns(self):
        destinations = ['http://www.nature.com/article', 'http://127.0.0.1/', 'https://127.0.0.1/',
                        'https://10.0.0.1/', 'https://[::1]/', 'file:///etc/passwd', 'ftp://doi.org/x',
                        'data:text/plain,test', 'https://doi.org:8443/x', 'https://doi.org:0/x',
                        'https://user:secret@doi.org/x', '//localhost/x', 'https://doi.org.evil.test/',
                        'https://2130706433/', 'https://127.1/', 'https://doi.org./x', 'https://doi.org\\@localhost/x']
        for destination in destinations:
            with self.subTest(destination=destination):
                network = Network([response(302, destination)])
                with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/start')
                self.assertEqual(network.resolved, ['doi.org'])
                self.assertEqual(len(network.connected), 1)

    def test_private_reserved_mixed_and_transition_dns_answers_never_connect(self):
        bad = ['127.0.0.1', '10.1.2.3', '172.16.0.1', '192.168.1.1', '169.254.169.254',
               '100.64.0.1', '0.0.0.0', '192.0.2.1', '198.18.0.1', '224.0.0.1', '240.0.0.1',
               '::1', '::', 'fc00::1', 'fe80::1', 'ff02::1', '2001:db8::1', '::ffff:8.8.8.8',
               '64:ff9b::a00:1', '2002:0a00:0001::1', '2001::1']
        for address in bad:
            with self.subTest(address=address):
                network = Network([response()], {'doi.org': ['93.184.216.34', address]})
                with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/test')
                self.assertEqual(network.connected, [])

    def test_redirect_to_allowlisted_host_with_private_dns_is_refused(self):
        network = Network([response(302, 'https://www.nature.com/x')], {'www.nature.com': ['192.168.1.10']})
        with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/test')
        self.assertEqual(len(network.connected), 1)
        self.assertEqual(network.resolved, ['doi.org', 'www.nature.com'])

    def test_same_host_dns_change_is_rechecked_for_each_redirect(self):
        addresses = iter([['93.184.216.34'], ['127.0.0.1']])
        network = Network([response(302, '/next')], {'doi.org': lambda: next(addresses)})
        with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/start')
        self.assertEqual(len(network.connected), 1)

    def test_public_native_ipv6_is_accepted(self):
        network = Network([response()], {'doi.org': ['2606:4700:4700::1111']})
        self.assertEqual(self.client(network).get('https://doi.org/test')['http_status'], 200)
        self.assertEqual(network.connected[0][1], ('2606:4700:4700::1111', 443, 0, 0))

    def test_peer_mismatch_and_insecure_tls_context_are_rejected(self):
        network = Network([response()]); network.peer = ('127.0.0.1', 443)
        with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/test')
        self.assertEqual(network.handshakes, [])
        network = Network([response()]); client = self.client(network)
        context = TLS(network); context.check_hostname = False
        with patch.object(ssl, 'create_default_context', return_value=context):
            with self.assertRaises(http.UnsafeDestination): client.get('https://doi.org/test')
        self.assertTrue(network.sockets[0].closed)

    def test_tls_certificate_failure_is_not_disabled(self):
        network = Network([response()]); client = self.client(network)
        with patch.object(Socket, 'do_handshake', side_effect=ssl.SSLCertVerificationError('Synthetic invalid certificate')):
            with self.assertRaises(ssl.SSLCertVerificationError): client.get('https://doi.org/test')
        self.assertTrue(all(s.closed for s in network.sockets))

    def test_watchdog_can_shutdown_a_socket_until_body_is_closed(self):
        network = Network([response(body=b'{"value":1}')]); client = self.client(network)
        original = http.http.client.HTTPResponse.read1
        def read(response, amount):
            self.assertFalse(network.sockets[0].closed)
            self.timers[-1].callback()
            self.assertTrue(network.sockets[0].shutdown_called)
            return original(response, amount)
        with patch.object(http.http.client.HTTPResponse, 'read1', read):
            with self.assertRaises(TimeoutError): client.get('https://doi.org/test', as_json=True)
        self.assertTrue(network.sockets[0].closed)

    def test_connection_fallback_is_limited_and_does_not_resolve_again(self):
        network = Network([], {'doi.org': ['93.184.216.34', '8.8.8.8', '1.1.1.1', '8.8.4.4', '1.0.0.1']})
        client = self.client(network)
        with patch.object(Socket, 'connect', side_effect=OSError('Synthetic unreachable endpoint')):
            with self.assertRaises(OSError): client.get('https://doi.org/a')
        self.assertEqual(network.resolved, ['doi.org'])
        self.assertEqual(len(network.sockets), 4)
        self.assertTrue(all(s.closed for s in network.sockets))

    def test_redirect_loops_fragments_excess_missing_and_duplicate_location(self):
        cases = [[response(302, '#section')], [response(302, '/b'), response(302, '/a')],
                 [response(302)], [response(302, '/b', extra='Location: /c\r\n')],
                 [response(305, '/proxy')]]
        for replies in cases:
            with self.subTest(replies=len(replies)):
                network = Network(replies)
                with self.assertRaises(http.UnsafeDestination): self.client(network).get('https://doi.org/a')
                self.assertLessEqual(len(network.connected), len(replies))

    def test_429_retains_retry_after_and_does_not_retry(self):
        network = Network([response(429, extra='Retry-After: 60\r\n')])
        with self.assertRaises(http.urllib.error.HTTPError) as result: self.client(network).get('https://doi.org/a')
        self.assertEqual(result.exception.headers['Retry-After'], '60')
        self.assertEqual(len(network.connected), 1)

    def test_json_size_link_prefix_and_deadline_limits(self):
        network = Network([response(body=b'x' * 4_000_001)])
        with self.assertRaises(ValueError): self.client(network).get('https://doi.org/json', as_json=True)
        network = Network([response(body=b'x' * 4096)])
        self.assertEqual(self.client(network).get('https://doi.org/html')['http_status'], 200)
        network = Network([response(302, '/next')])
        with self.assertRaises(TimeoutError): self.client(network, total_timeout=1).get('https://doi.org/a')
        self.assertEqual(len(network.connected), 1)

    def test_lpsn_restriction_and_malformed_urls_fail_without_dns(self):
        network = Network([]); client = self.client(network)
        for url in ('https://lpsn.dsmz.de/genus/example', 'https://www.lpsn.dsmz.de/genus/example'):
            with self.assertRaises(PermissionError): client.get(url)
        for url in ('https://doi.org/\nheader', 'https://doi.org:bad/x', 'https://doi.org/a b'):
            with self.assertRaises(http.UnsafeDestination): client.get(url)
        self.assertEqual(network.resolved, [])

    def test_dns_deadline_has_no_unbounded_worker_accumulation(self):
        release = threading.Event(); entered = threading.Event()
        def resolver(*args):
            entered.set(); release.wait(2); return [answer()]
        dns = http.PublicResolver(resolver)
        try:
            with self.assertRaises(TimeoutError): dns.resolve('doi.org', .02)
            self.assertTrue(entered.is_set())
            with self.assertRaisesRegex(TimeoutError, 'pending'): dns.resolve('doi.org', .02)
        finally:
            release.set()

    def test_policy_covers_existing_references_without_automatically_trusting_new_hosts(self):
        from content_governance import records
        urls = {r['source_url'] for r in records(ROOT).values() if r.get('source_url')}
        urls.update(r['url'] for r in json.loads((ROOT/'resources/data/phylogeny.json').read_text(encoding='utf-8'))['sources'].values())
        photos = json.loads((ROOT/'resources/data/exploration_photos.json').read_text(encoding='utf-8'))['photos']
        urls.update(p[k] for p in photos.values() for k in ('photo_url', 'license_url'))
        for url in urls:
            try: http.DestinationPolicy().validate(url)
            except PermissionError: self.assertIn('lpsn.dsmz.de', url)
        with self.assertRaises(http.UnsafeDestination): http.DestinationPolicy().validate('https://new-source.example/paper')

    def test_environment_proxy_cannot_replace_the_validated_endpoint(self):
        network = Network([response()]); client = self.client(network)
        with patch.dict('os.environ', {'HTTPS_PROXY': 'http://127.0.0.1:8080', 'ALL_PROXY': 'http://10.0.0.1:8080'}):
            self.assertEqual(client.get('https://doi.org/a')['http_status'], 200)
        self.assertEqual(network.connected, [('doi.org', ('93.184.216.34', 443))])


if __name__ == '__main__':
    unittest.main()
