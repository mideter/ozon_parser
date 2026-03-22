#include "product.h"


Product::Product(int index, const QString& name, int price,
                 int reviewPoints, const QString& url)
    : index_(index)
    , name_(name)
    , price_(price)
    , reviewPoints_(reviewPoints)
    , url_(url)
{}


int Product::index() const 
{ 
    return index_; 
}


QString Product::name() const 
{ 
    return name_; 
}


int Product::price() const
{ 
    return price_;
}


int Product::reviewPoints() const 
{ 
    return reviewPoints_; 
}


QString Product::url() const 
{ 
    return url_; 
}


double Product::pointsToPriceRatio() const
{
    if (price_ <= 0)
        return 0.0;
    return static_cast<double>(reviewPoints_) / static_cast<double>(price_);
}
