#!/usr/bin/env python3
"""
Тестовый скрипт для проверки, пропускает ли антибот Ozon запросы Selenium.
Загружает страницу категории Ozon и выводит: успех (найдены товары) или блокировка.
"""

import argparse
import os
import sys
import time


def run_selenium_test(url: str, headless: bool) -> int:
    """Запуск теста с обычным Selenium."""
    from selenium import webdriver
    from selenium.webdriver.chrome.options import Options
    from selenium.webdriver.chrome.service import Service
    from selenium.webdriver.support.ui import WebDriverWait

    opts = Options()
    if headless:
        opts.add_argument("--headless=new")
    opts.add_argument(
        "user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    )
    opts.add_argument("--disable-blink-features=AutomationControlled")

    # Системный Chromium (apt: chromium, chromium-driver) — без Selenium Manager
    for binary in ("/usr/bin/chromium", "/usr/bin/chromium-browser", "/usr/bin/google-chrome"):
        if os.path.isfile(binary):
            opts.binary_location = binary
            break
    chromedriver = "/usr/bin/chromedriver"
    if not os.path.isfile(chromedriver):
        print("Ошибка: chromedriver не найден. Установите: sudo apt install chromium chromium-driver")
        return 3
    service = Service(executable_path=chromedriver)
    driver = webdriver.Chrome(service=service, options=opts)
    try:
        driver.get(url)
        WebDriverWait(driver, 15).until(
            lambda d: d.execute_script("return document.readyState") == "complete"
        )
        time.sleep(3)
        return check_page(driver)
    finally:
        driver.quit()


def run_undetected_test(url: str, headless: bool) -> int:
    """Запуск теста с undetected-chromedriver."""
    try:
        import undetected_chromedriver as uc
    except ImportError:
        print(
            "Ошибка: undetected-chromedriver не установлен.\n"
            "  pip install undetected-chromedriver\n"
            "  или: sudo apt install python3-undetected-chromedriver\n"
            "  (требует доступ к PyPI для pip). Без него используйте обычный Selenium."
        )
        return 3

    options = uc.ChromeOptions()
    if headless:
        options.add_argument("--headless=new")

    driver = uc.Chrome(options=options)
    try:
        driver.get(url)
        driver.implicitly_wait(10)
        return check_page(driver)
    finally:
        driver.quit()


def check_page(driver) -> int:
    """
    Проверяет страницу на успех/блокировку.
    Returns: 0 = успех, 1 = блокировка, 2 = неопределённо
    """
    title = driver.title or ""
    body_text = ""
    try:
        body = driver.find_element("tag name", "body")
        body_text = body.text[:500] if body else ""
    except Exception:
        pass

    # Проверка блокировки
    if "Доступ ограничен" in title or "доступ ограничен" in body_text.lower():
        print("Блокировка: Ozon ограничивает доступ (anti-bot сработал)")
        return 1

    # Проверка успеха — наличие товарных tile
    tiles = driver.find_elements("css selector", "div.tile-root[data-index]")
    if tiles:
        print(f"OK: найдено {len(tiles)} товаров")
        return 0

    paginator = driver.find_elements("css selector", "#contentScrollPaginator")
    if paginator:
        items = driver.find_elements("css selector", "div.pi5_24")
        if items:
            print(f"OK: найден пагинатор, {len(items)} элементов")
            return 0

    # Неуверенный результат
    print("Неопределённо: товары не найдены, явной блокировки нет.")
    print(f"  Title: {title[:80]}")
    if body_text:
        print(f"  Body (фрагмент): {body_text[:200]}...")
    return 2


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Проверка, пропускает ли антибот Ozon запросы Selenium"
    )
    parser.add_argument(
        "url",
        nargs="?",
        default="https://www.ozon.ru/category/knigi-16500/",
        help="URL страницы Ozon (категория или поиск)",
    )
    parser.add_argument(
        "--undetected",
        action="store_true",
        help="Использовать undetected-chromedriver",
    )
    parser.add_argument(
        "--headless",
        action="store_true",
        help="Запуск в headless-режиме",
    )
    args = parser.parse_args()

    print(f"URL: {args.url}")
    print(f"Режим: {'undetected-chromedriver' if args.undetected else 'Selenium'}")
    print(f"Headless: {args.headless}")
    print("---")

    if args.undetected:
        return run_undetected_test(args.url, args.headless)
    return run_selenium_test(args.url, args.headless)


if __name__ == "__main__":
    sys.exit(main())
