#pragma once

#include <QString>

#include <optional>

struct ParsedTile {
    QString name;
    int price = 0;
    int reviewPoints = 0;

    /** Разбор полей карточки из outerHTML плитки Ozon (перенесённые и адаптированные эвристики Python-парсера). */
    static std::optional<ParsedTile> parseHtml(const QString& html);
};
