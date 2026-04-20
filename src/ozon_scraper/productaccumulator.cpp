#include "ozon_scraper/productaccumulator.h"

void ProductAccumulator::reset()
{
    seenUrls_.clear();
    allProducts_.clear();
    lastPrice_ = 0;
}

ProductAccumulatorAddResult ProductAccumulator::addBatch(const QVector<Product>& batch)
{
    ProductAccumulatorAddResult result;

    for (const Product& product : batch) {
        const QString url = product.url();

        if (seenUrls_.contains(url))
            continue;

        seenUrls_.insert(url);
        allProducts_.append(product);
        ++result.addedCount;

        if (product.price() > 0)
            lastPrice_ = product.price();
    }

    result.totalCount = allProducts_.size();
    result.lastPrice = lastPrice_;
    return result;
}

const QVector<Product>& ProductAccumulator::allProducts() const
{
    return allProducts_;
}

int ProductAccumulator::totalCount() const
{
    return allProducts_.size();
}

int ProductAccumulator::lastPrice() const
{
    return lastPrice_;
}
