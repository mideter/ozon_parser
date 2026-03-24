#!/usr/bin/env python3
"""
Загрузка страницы Ozon через undetected-chromedriver, скролл и извлечение товаров.
Пишет в stdout NDJSON: {"type":"batch","items":[...]}, затем завершает процесс с кодом 0.
Ошибки: {"type":"error","message":"..."} и exit 1.
"""

from __future__ import annotations

import argparse
import json
import sys
import time

# Совпадает с ozonscraper.cpp
EXTRA_WAIT_MS = 500
SCROLL_DELAY_MS = 120
SCROLL_TIMER_MS = 150

JS_WAIT_PRODUCTS = """
(function() {
    var paginator = document.getElementById('contentScrollPaginator');
    if (!paginator) return false;
    var items = paginator.querySelectorAll('div.pi5_24');
    return items.length > 0;
})()
"""

JS_EXTRACT_PRODUCTS = """
(function() {
    var results = [];
    var tiles = document.querySelectorAll('div.tile-root[data-index]');
    for (var i = 0; i < tiles.length; i++) {
        var el = tiles[i];
        var name = '';
        try {
            var nameEl = el.querySelector("a[target='_blank'] > div > span");
            if (nameEl) name = nameEl.textContent.trim();
        } catch(e) {}
        if (name.length < 3) continue;

        var price = 0;
        try {
            var priceEl = el.querySelector("> div > div > div > span");
            if (priceEl) {
                var m = priceEl.textContent.match(/[\\d\\s\\u2009,]+/g);
                if (m && m.length > 0) {
                    var last = m[m.length - 1].replace(/[\\s\\u2009,]/g, '');
                    price = parseInt(last, 10) || 0;
                }
            }
        } catch(e) {}

        var points = 0;
        try {
            var pointsEl = el.querySelector("section div[title]");
            if (pointsEl) {
                var m = pointsEl.textContent.match(/(\\d+(?:\\s+\\d+)*)/);
                if (m) {
                    var s = m[1].replace(/\\s/g, '');
                    points = parseInt(s, 10) || 0;
                }
            }
        } catch(e) {}

        var url = '';
        try {
            var linkEl = el.querySelector("> a");
            if (linkEl && linkEl.href && linkEl.href.indexOf('/product/') >= 0) {
                url = linkEl.href.split('?')[0].split('#')[0].replace(/\\/$/, '');
                if (url.indexOf('/reviews') >= 0 || url.indexOf('/questions') >= 0 || url.indexOf('/seller') >= 0)
                    url = '';
            }
        } catch(e) {}
        if (!url || url.split('/product/').pop().split('/')[0].length < 3) continue;

        results.push({name: name, price: price, review_points: points, url: url});
    }
    return JSON.stringify(results);
})()
"""

JS_SCROLL = """
(function() {
    window.scrollTo(0, document.body.scrollHeight);
    return document.body.scrollHeight;
})()
"""

JS_GET_HEIGHT = "(document.body.scrollHeight)"


def emit_error(msg: str) -> None:
    print(json.dumps({"type": "error", "message": msg}, ensure_ascii=False), flush=True)
    sys.exit(1)


def emit_batch(items: list) -> None:
    if not items:
        return
    print(
        json.dumps({"type": "batch", "items": items}, ensure_ascii=False),
        flush=True,
    )


def run(url: str, headless: bool) -> None:
    try:
        import undetected_chromedriver as uc
    except ImportError:
        emit_error(
            "Не установлен undetected-chromedriver. "
            "Установите: sudo apt install python3-undetected-chromedriver"
        )

    options = uc.ChromeOptions()
    if headless:
        options.add_argument("--headless=new")

    driver = uc.Chrome(options=options)
    try:
        driver.get(url)
        time.sleep(0.5)
        title = driver.title or ""
        body_text = ""
        try:
            body = driver.find_element("tag name", "body")
            body_text = (body.text or "")[:500]
        except Exception:
            pass
        if "Доступ ограничен" in title or "доступ ограничен" in body_text.lower():
            emit_error(
                "Ozon ограничивает доступ с вашей сети. Попробуйте: отключить VPN, "
                "подключиться к другой сети (Wi‑Fi/мобильный интернет) или перезагрузить роутер."
            )

        for _ in range(60):
            if driver.execute_script("return document.readyState") == "complete":
                break
            time.sleep(0.1)
        time.sleep(3)

        ok = driver.execute_script(JS_WAIT_PRODUCTS)
        if not ok:
            time.sleep(0.3)
            ok = driver.execute_script(JS_WAIT_PRODUCTS)
        if not ok:
            emit_error("Товары не найдены. Проверьте URL и структуру страницы Ozon.")

        last_height = 0
        while True:
            driver.execute_script(JS_SCROLL)
            time.sleep(SCROLL_DELAY_MS / 1000.0)
            new_height = int(driver.execute_script(JS_GET_HEIGHT))
            raw = driver.execute_script(JS_EXTRACT_PRODUCTS)
            items = json.loads(raw) if raw else []
            emit_batch(items)

            if new_height == last_height:
                time.sleep(EXTRA_WAIT_MS / 1000.0)
                h2 = int(driver.execute_script(JS_GET_HEIGHT))
                if h2 != last_height:
                    last_height = h2
                    time.sleep(SCROLL_TIMER_MS / 1000.0)
                    continue
                break
            last_height = new_height
            time.sleep(SCROLL_TIMER_MS / 1000.0)

        print(json.dumps({"type": "done"}, ensure_ascii=False), flush=True)
    finally:
        driver.quit()


def main() -> int:
    parser = argparse.ArgumentParser(description="Ozon fetch для Qt (NDJSON в stdout)")
    parser.add_argument("url", help="URL категории или поиска Ozon")
    parser.add_argument("--headless", action="store_true", help="Headless Chromium")
    args = parser.parse_args()
    try:
        run(args.url, args.headless)
    except BrokenPipeError:
        return 0
    except SystemExit:
        raise
    except Exception as e:
        emit_error(str(e))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
