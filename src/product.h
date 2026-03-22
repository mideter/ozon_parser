#pragma once

#include <QString>


class Product
{
public:
    Product(int index = 0,
           const QString& name = {},
           int price = 0,
           int reviewPoints = 0,
           const QString& url = {});

    int index() const;
    QString name() const;
    int price() const;
    int reviewPoints() const;
    QString url() const;

    double pointsToPriceRatio() const;

private:
    int index_ = 0;
    QString name_;
    int price_ = 0;
    int reviewPoints_ = 0;
    QString url_;
};
