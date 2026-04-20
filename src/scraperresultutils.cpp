#include "scraperresultutils.h"

#include <algorithm>

#include <QtGlobal>


QVector<Product> ScraperResultUtils::computeTopProducts(const QVector<Product>& allProducts,
                                                        int minPoints,
                                                        int maxPoints,
                                                        int limit)
{
    QVector<Product> filtered;
    filtered.reserve(allProducts.size());

    for (const Product& product : allProducts) {
        const int points = product.reviewPoints();

        if (minPoints >= 0 && points < minPoints)
            continue;
        if (maxPoints >= 0 && points > maxPoints)
            continue;

        filtered.append(product);
    }

    std::sort(filtered.begin(), filtered.end(), [](const Product& lhs, const Product& rhs) {
        return lhs.pointsToPriceRatio() > rhs.pointsToPriceRatio();
    });

    const int resultSize = qMin(limit, filtered.size());
    QVector<Product> topProducts;
    topProducts.reserve(resultSize);

    for (int i = 0; i < resultSize; ++i) {
        const Product& original = filtered[i];
        topProducts.append(
            Product(i + 1, original.name(), original.price(), original.reviewPoints(), original.url()));
    }

    return topProducts;
}


QString ScraperResultUtils::formatElapsedText(qint64 elapsedMs)
{
    const double seconds = elapsedMs / 1000.0;

    if (seconds < 60.0)
        return QStringLiteral("Затрачено %1 сек").arg(seconds, 0, 'f', 1);

    const int minutes = static_cast<int>(seconds / 60.0);
    const double secondsRemainder = seconds - minutes * 60.0;

    if (secondsRemainder < 0.1)
        return QStringLiteral("Затрачено %1 мин").arg(minutes);

    return QStringLiteral("Затрачено %1 мин %2 сек").arg(minutes).arg(secondsRemainder, 0, 'f', 1);
}
