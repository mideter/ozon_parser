#pragma once

#include <QString>

#include <optional>

struct ParsedTile {
    QString name;
    int price = 0;
    int reviewPoints = 0;
};

/** Разбор полей карточки из outerHTML плитки Ozon (эвристики как в scripts/product_parser.py). */
std::optional<ParsedTile> parseOzonTileHtml(const QString& html, const QString& url);
