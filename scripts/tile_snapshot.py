"""Извлечение outerHTML и href плитки товара без полного ProductParser."""
from __future__ import annotations

from typing import Any, Dict, Optional

from url_normalizer import URLNormalizer

# Один round-trip: outerHTML + первый подходящий /product/ URL
JS_TILE_OUTER_AND_HREF = """
return (function(el) {
    var a = null;
    try {
        if (el.matches && el.matches('a[href*="/product/"]')) a = el;
    } catch (e) {}
    if (!a) a = el.querySelector('a[href*="/product/"]');
    if (!a && el.tagName === 'A') {
        var h = el.getAttribute('href') || '';
        if (h.indexOf('/product/') >= 0) a = el;
    }
    var href = (a && a.href) ? a.href : '';
    return { outerHTML: el.outerHTML || '', href: href };
})(arguments[0]);
"""


def tile_html_and_url(driver, element) -> Optional[Dict[str, str]]:
    """Возвращает {'html': outerHTML, 'url': нормализованный URL} или None если URL невалиден."""
    try:
        raw: Any = driver.execute_script(JS_TILE_OUTER_AND_HREF, element)
    except Exception:
        return None
    if not raw or not isinstance(raw, dict):
        return None
    html = raw.get("outerHTML") or ""
    href = raw.get("href") or ""
    normalized = URLNormalizer.normalize(href)
    if not URLNormalizer.is_valid(normalized):
        return None
    return {"html": html, "url": normalized}
