"""Access restrictions shared with legacy maintenance helpers."""
from urllib.parse import urlparse


def access_issue(url):
    host=urlparse(url).hostname or ''
    if (host=='lpsn.dsmz.de' or host.endswith('.lpsn.dsmz.de')) and host!='api.lpsn.dsmz.de':
        return 'LPSN prohibits automated HTML downloads: use the official API/permitted download workflow after configuration. https://lpsn.dsmz.de/text/copyright'
    return None
