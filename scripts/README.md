# scripts / ozon_fetch

`ozon_fetch.py` загружает страницу (`PageLoader`), находит плитки (`ElementFinder`), скроллит (`Scroller`), для каждой новой карточки отправляет в stdout **нормализованный URL** и **outerHTML** узла плитки. Драйвер — **undetected-chromedriver**. Вывод для Qt — NDJSON:

- `{"type":"batch","items":[{"url":"https://...","html":"<div class=\"tile-root\" ...>...</div>"}, ...]}`
- затем `{"type":"done"}` или `{"type":"error","message":"..."}`

Разбор названия, цены и баллов выполняется в приложении (`src/productcardparser.cpp`), не в Python.

Можно передать несколько URL подряд — браузер поднимается один раз, страницы обрабатываются последовательно.

```bash
pip install -r requirements.txt
python3 ozon_fetch.py "https://www.ozon.ru/..." "https://www.ozon.ru/..."
```
