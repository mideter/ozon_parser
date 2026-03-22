# Ozon Scraper (Qt)

Парсер товаров Ozon на чистом Qt без Python. Использует Qt WebEngine (Chromium) для рендеринга страниц и извлечения данных через JavaScript.

## Зависимости (Linux)

```bash
# Debian/Ubuntu — сборка
sudo apt install qtbase5-dev qtdeclarative5-dev qtwebengine5-dev qtquickcontrols2-5-dev

# Debian/Ubuntu — QML‑модули для запуска
sudo apt install qml-module-qtquick-controls2 qml-module-qtquick-layouts qml-module-qtquick-window2

# При ошибке "QQml_colorProvider" или "Unable to assign undefined to QColor":
sudo apt install qt5-quick-demos
```

## Сборка

```bash
mkdir build && cd build
cmake ..
make
```

## Запуск

```bash
./ozon_cpp
```

## Возможности

- Загрузка страниц категорий и поиска Ozon
- Автоматический скролл для подгрузки всех товаров
- Фильтрация по баллам за отзыв (мин/макс)
- Топ-50 товаров по соотношению баллы/цена
- Обновление таблицы в реальном времени
