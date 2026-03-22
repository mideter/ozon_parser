#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "product.h"


class ProductModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)

public:
    enum Role {
        IndexRole = Qt::UserRole + 1,
        NameRole,
        PriceRole,
        ReviewPointsRole,
        RatioRole,
        UrlRole
    };

    explicit ProductModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setProducts(const QVector<Product>& products, int totalCount = -1);
    Q_INVOKABLE void clear();
    const QVector<Product>& products() const { return products_; }
    int count() const { return products_.size(); }
    int totalCount() const { return totalCount_; }

signals:
    void countChanged();
    void totalCountChanged();

private:
    QVector<Product> products_;
    int totalCount_ = 0;
};
