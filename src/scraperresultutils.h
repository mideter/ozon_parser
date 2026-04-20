#pragma once

#include "product.h"

#include <QVector>
#include <QString>


class ScraperResultUtils
{
public:
    static QVector<Product> computeTopProducts(const QVector<Product>& allProducts,
                                               int minPoints,
                                               int maxPoints,
                                               int limit = 50);

    static QString formatElapsedText(qint64 elapsedMs);
};
