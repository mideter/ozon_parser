#pragma once

#include "product.h"

#include <QSet>
#include <QVector>

struct ProductAccumulatorAddResult
{
    int addedCount = 0;
    int totalCount = 0;
    int lastPrice = 0;
};

class ProductAccumulator
{
public:
    void reset();
    ProductAccumulatorAddResult addBatch(const QVector<Product>& batch);

    const QVector<Product>& allProducts() const;
    int totalCount() const;
    int lastPrice() const;

private:
    QSet<QString> seenUrls_;
    QVector<Product> allProducts_;
    int lastPrice_ = 0;
};
