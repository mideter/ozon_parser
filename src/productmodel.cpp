#include "productmodel.h"


ProductModel::ProductModel(QObject* parent)
    : QAbstractListModel(parent)
{}


int ProductModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return products_.size();
}


QVariant ProductModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= products_.size())
        return {};

    const Product& p = products_.at(index.row());

    switch (role) {
    case IndexRole:
        return p.index();
    case NameRole:
        return p.name();
    case PriceRole:
        return p.price();
    case ReviewPointsRole:
        return p.reviewPoints();
    case RatioRole: {
        const double ratio = p.pointsToPriceRatio();

        if (ratio <= 0.0)
            return QStringLiteral("—");
        
        return QString::number(ratio, 'f', 4);
    }
    case UrlRole:
        return p.url();
    default:
        return {};
    }
}


QHash<int, QByteArray> ProductModel::roleNames() const
{
    return {
        { IndexRole, "index" },
        { NameRole, "name" },
        { PriceRole, "price" },
        { ReviewPointsRole, "reviewPoints" },
        { RatioRole, "ratio" },
        { UrlRole, "url" }
    };
}


void ProductModel::setProducts(const QVector<Product>& products, int totalCount)
{
    beginResetModel();
    products_ = products;
    endResetModel();
    if (totalCount >= 0) {
        if (totalCount_ != totalCount) {
            totalCount_ = totalCount;
            emit totalCountChanged();
        }
    }
    emit countChanged();
}


void ProductModel::clear()
{
    if (totalCount_ != 0) {
        totalCount_ = 0;
        emit totalCountChanged();
    }
    setProducts({}, -1);
}
