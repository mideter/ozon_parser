#pragma once

#include "product.h"

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QSet>
#include <QUrl>
#include <QVector>
#include <QWebEnginePage>


class OzonScraper : public QObject
{
    Q_OBJECT

public:
    explicit OzonScraper(QObject* parent = nullptr);
    ~OzonScraper() override;

    void start(const QUrl& url, int minPoints, int maxPoints);
    Q_INVOKABLE void start(const QString& urlStr, int minPoints, int maxPoints);
    void stop();

signals:
    void statusChanged(const QString& message, int count, int lastPrice);
    void topProductsUpdated(const QVector<Product>& products, int totalCount);
    void finishedSuccessfully(int totalCount, const QString& elapsedText);
    void finishedWithError(const QString& message);

private slots:
    void onLoadFinished(bool ok);
    void onScrollAndExtract();
    void onExtractResult(const QVariant& result);

private:
    void finishWithError(const QString& message);
    void finishWithSuccess();
    QString formatElapsed(qint64 ms) const;
    QVector<Product> parseProductsFromJson(const QByteArray& json);
    QVector<Product> computeTop50(const QVector<Product>& all) const;

    static constexpr int UPDATE_TABLE_EVERY_N = 15;
    static constexpr int EXTRA_WAIT_MS = 500;
    static const char* const JS_WAIT_PRODUCTS;
    static const char* const JS_EXTRACT_PRODUCTS;
    static const char* const JS_SCROLL;
    static const char* const JS_GET_HEIGHT;

    QWebEnginePage* page_ = nullptr;
    QTimer* scrollTimer_ = nullptr;
    QElapsedTimer elapsedTimer_;
    bool running_ = false;

    QUrl url_;
    int minPoints_ = -1;
    int maxPoints_ = -1;
    QSet<QString> seenUrls_;
    QVector<Product> allProducts_;
    int lastTableCount_ = 0;
    int lastHeight_ = 0;
    int lastPrice_ = 0;
};
