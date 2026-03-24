# Selenium Anti-Bot Test

Тестовый скрипт для проверки, пропускает ли антибот Ozon запросы Selenium.

## Зависимости

- Python 3.8+
- Google Chrome или Chromium
- chromedriver

## Установка

### Вариант A: через apt (если PyPI недоступен)

```bash
sudo apt install python3-selenium chromium chromium-driver
```

Для режима `--undetected` (если обычный Selenium блокируется):

```bash
sudo apt install python3-undetected-chromedriver
```

Скрипт работает с системным Python, venv не нужен.

### Вариант B: через pip (venv)

```bash
sudo apt install python3-venv
cd selenium_test
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
```

При таймаутах PyPI: `pip install --timeout 120 -r requirements.txt` или другое зеркало.

## Запуск

**Если установили через apt (вариант A):**

```bash
cd selenium_test
python3 test_antibot.py "https://www.ozon.ru/category/knigi-16500/"
```

**Если через venv (вариант B):**

```bash
cd selenium_test
.venv/bin/python test_antibot.py "https://www.ozon.ru/category/knigi-16500/"
```

Обычный Selenium (по умолчанию, видимый браузер) — см. выше.

С undetected-chromedriver (если обычный Selenium блокируется):

```bash
python3 test_antibot.py --undetected "https://www.ozon.ru/category/knigi-16500/"
```

Headless-режим:

```bash
python3 test_antibot.py --headless "https://www.ozon.ru/category/knigi-16500/"
```

## Результаты

| Вывод | Код возврата | Значение |
|-------|--------------|----------|
| `OK: найдено N товаров` | 0 | Антибот пропустил |
| `Блокировка: Ozon ограничивает доступ` | 1 | Anti-bot сработал — попробуйте `--undetected` |
| `Неопределённо: ...` | 2 | Товары не найдены, явной блокировки нет |

Если обычный Selenium блокируется, установите и используйте undetected-chromedriver: `sudo apt install python3-undetected-chromedriver`

## URL по умолчанию

Если URL не указан, используется `https://www.ozon.ru/category/knigi-16500/`.
