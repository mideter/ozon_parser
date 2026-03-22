#include "ozonscraper.h"

#include <algorithm>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebEngineProfile>

namespace {

QString normalizeUrl(const QString& href)
{
    if (href.isEmpty())
        return {};
    QString u = href.split(QLatin1Char('?')).first().split(QLatin1Char('#')).first();
    while (u.endsWith(QLatin1Char('/')))
        u.chop(1);
    return u;
}

bool isValidProductUrl(const QString& url)
{
    if (url.isEmpty() || !url.contains(QLatin1String("/product/")))
        return false;
    if (url.contains(QLatin1String("/reviews")) || url.contains(QLatin1String("/questions"))
        || url.contains(QLatin1String("/seller")))
        return false;
    const int idx = url.indexOf(QLatin1String("/product/")) + 9;
    const QString id = url.mid(idx).split(QLatin1Char('/')).first();
    return id.length() >= 3;
}

} // namespace

const char* const OzonScraper::JS_WAIT_PRODUCTS = R"(
(function() {
    var paginator = document.getElementById('contentScrollPaginator');
    if (!paginator) return false;
    var items = paginator.querySelectorAll('div.pi5_24');
    return items.length > 0;
})()
)";

const char* const OzonScraper::JS_EXTRACT_PRODUCTS = R"(
(function() {
    var results = [];
    var tiles = document.querySelectorAll('div.tile-root[data-index]');
    for (var i = 0; i < tiles.length; i++) {
        var el = tiles[i];
        var name = '';
        try {
            var nameEl = el.querySelector("a[target='_blank'] > div > span");
            if (nameEl) name = nameEl.textContent.trim();
        } catch(e) {}
        if (name.length < 3) continue;

        var price = 0;
        try {
            var priceEl = el.querySelector("> div > div > div > span");
            if (priceEl) {
                var m = priceEl.textContent.match(/[\d\s\u2009,]+/g);
                if (m && m.length > 0) {
                    var last = m[m.length - 1].replace(/[\s\u2009,]/g, '');
                    price = parseInt(last, 10) || 0;
                }
            }
        } catch(e) {}

        var points = 0;
        try {
            var pointsEl = el.querySelector("section div[title]");
            if (pointsEl) {
                var m = pointsEl.textContent.match(/(\d+(?:\s+\d+)*)/);
                if (m) {
                    var s = m[1].replace(/\s/g, '');
                    points = parseInt(s, 10) || 0;
                }
            }
        } catch(e) {}

        var url = '';
        try {
            var linkEl = el.querySelector("> a");
            if (linkEl && linkEl.href && linkEl.href.indexOf('/product/') >= 0) {
                url = linkEl.href.split('?')[0].split('#')[0].replace(/\/$/, '');
                if (url.indexOf('/reviews') >= 0 || url.indexOf('/questions') >= 0 || url.indexOf('/seller') >= 0)
                    url = '';
            }
        } catch(e) {}
        if (!url || url.split('/product/').pop().split('/')[0].length < 3) continue;

        results.push({name: name, price: price, review_points: points, url: url});
    }
    return JSON.stringify(results);
})()
)";

const char* const OzonScraper::JS_SCROLL = R"(
(function() {
    window.scrollTo(0, document.body.scrollHeight);
    return document.body.scrollHeight;
})()
)";

const char* const OzonScraper::JS_GET_HEIGHT = R"(
(document.body.scrollHeight)
)";


OzonScraper::OzonScraper(QObject* parent)
    : QObject(parent)
    , scrollTimer_(new QTimer(this))
{
    scrollTimer_->setSingleShot(true);
    connect(scrollTimer_, &QTimer::timeout, this, &OzonScraper::onScrollAndExtract);
}


OzonScraper::~OzonScraper()
{
    stop();
    if (page_) {
        page_->deleteLater();
        page_ = nullptr;
    }
}


void OzonScraper::start(const QString& urlStr, int minPoints, int maxPoints)
{
    const QUrl url = QUrl::fromUserInput(urlStr);
    if (url.isValid())
        start(url, minPoints, maxPoints);
    else
        emit finishedWithError(QStringLiteral("Некорректный URL."));
}


void OzonScraper::start(const QUrl& url, int minPoints, int maxPoints)
{
    if (running_)
        return;

    if (page_) {
        page_->deleteLater();
        page_ = nullptr;
    }

    url_ = url;
    minPoints_ = minPoints;
    maxPoints_ = maxPoints;
    seenUrls_.clear();
    allProducts_.clear();
    lastTableCount_ = 0;
    lastHeight_ = 0;
    lastPrice_ = 0;
    running_ = true;
    elapsedTimer_.start();

    QWebEngineProfile* profile = QWebEngineProfile::defaultProfile();
    profile->setHttpUserAgent(QStringLiteral(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"));
    page_ = new QWebEnginePage(profile, this);
    connect(page_, &QWebEnginePage::loadFinished, this, &OzonScraper::onLoadFinished);

    emit statusChanged(QStringLiteral("Загрузка страницы..."), -1, 0);
    page_->load(url);
}


void OzonScraper::stop()
{
    running_ = false;
    scrollTimer_->stop();
    if (page_) {
        page_->deleteLater();
        page_ = nullptr;
    }
}


void OzonScraper::onLoadFinished(bool ok)
{
    if (!running_ || !page_)
        return;

    if (!ok) {
        page_->runJavaScript(QStringLiteral(
            "(function(){var t=document.title||'';var b=document.body?document.body.innerText.substring(0,400):'';"
            "return JSON.stringify({title:t,bodySnippet:b});})()"),
            [this](const QVariant& v) {
                QString errMsg = QStringLiteral("Не удалось загрузить страницу.");
                if (!v.toString().isEmpty()) {
                    QJsonParseError err;
                    QJsonDocument doc = QJsonDocument::fromJson(v.toString().toUtf8(), &err);
                    if (err.error == QJsonParseError::NoError && doc.isObject()) {
                        QString title = doc.object().value(QLatin1String("title")).toString();
                        if (title.contains(QLatin1String("Доступ ограничен"))) {
                            errMsg = QStringLiteral(
                                "Ozon ограничивает доступ с вашей сети. Попробуйте: отключить VPN, "
                                "подключиться к другой сети (Wi‑Fi/мобильный интернет) или перезагрузить роутер.");
                        }
                    }
                }
                if (running_)
                    finishWithError(errMsg);
            });
        return;
    }

    emit statusChanged(QStringLiteral("Ожидание товаров..."), -1, 0);

    page_->runJavaScript(QLatin1String(JS_WAIT_PRODUCTS), [this](const QVariant& v) {
        if (!running_ || !page_)
            return;
        if (!v.toBool()) {
            QTimer::singleShot(300, this, [this]() {
                if (!running_ || !page_)
                    return;
                page_->runJavaScript(QLatin1String(JS_WAIT_PRODUCTS), [this](const QVariant& v2) {
                    if (!running_ || !page_)
                        return;
                    if (!v2.toBool()) {
                        finishWithError(QStringLiteral("Товары не найдены. Проверьте URL и структуру страницы Ozon."));
                        return;
                    }
                    lastHeight_ = 0;
                    scrollTimer_->start(150);
                });
            });
            return;
        }
        lastHeight_ = 0;
        scrollTimer_->start(150);
    });
}


void OzonScraper::onScrollAndExtract()
{
    if (!running_ || !page_)
        return;

    page_->runJavaScript(QLatin1String(JS_SCROLL), [this](const QVariant&) {
        if (!running_ || !page_)
            return;
        QTimer::singleShot(120, this, [this]() {
            if (!running_ || !page_)
                return;
            page_->runJavaScript(QLatin1String(JS_GET_HEIGHT), [this](const QVariant& heightVar) {
                if (!running_ || !page_)
                    return;
                int newHeight = heightVar.toInt();
                page_->runJavaScript(QLatin1String(JS_EXTRACT_PRODUCTS), [this, newHeight](const QVariant& result) {
                    onExtractResult(result);
                    if (!running_ || !page_)
                        return;

                    if (newHeight == lastHeight_) {
                        QTimer::singleShot(EXTRA_WAIT_MS, this, [this]() {
                            if (!running_ || !page_)
                                return;
                            page_->runJavaScript(QLatin1String(JS_GET_HEIGHT), [this](const QVariant& v2) {
                                int h2 = v2.toInt();
                                if (h2 != lastHeight_) {
                                    lastHeight_ = h2;
                                    scrollTimer_->start(150);
                                } else {
                                    finishWithSuccess();
                                }
                            });
                        });
                    } else {
                        lastHeight_ = newHeight;
                        scrollTimer_->start(150);
                    }
                });
            });
        });
    });
}


void OzonScraper::onExtractResult(const QVariant& result)
{
    const QByteArray json = result.toString().toUtf8();
    if (json.isEmpty())
        return;

    const QVector<Product> batch = parseProductsFromJson(json);
    int added = 0;
    for (const Product& p : batch) {
        const QString u = p.url();
        if (u.isEmpty() || seenUrls_.contains(u))
            continue;
        seenUrls_.insert(u);
        allProducts_.append(p);
        added++;
        if (p.price() > 0)
            lastPrice_ = p.price();
    }

    if (added > 0) {
        const int n = allProducts_.size();
        if (n == 1 || n - lastTableCount_ >= UPDATE_TABLE_EVERY_N) {
            lastTableCount_ = n;
            const QVector<Product> top = computeTop50(allProducts_);
            emit statusChanged(QStringLiteral("Найдено товаров"), n, lastPrice_);
            emit topProductsUpdated(top, n);
        }
    }
}


QVector<Product> OzonScraper::parseProductsFromJson(const QByteArray& json)
{
    QVector<Product> out;
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray())
        return out;

    const QJsonArray arr = doc.array();
    int index = allProducts_.size() + 1;
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        const QString name = o.value(QLatin1String("name")).toString().trimmed();
        if (name.length() < 3)
            continue;
        const int price = o.value(QLatin1String("price")).toInt(0);
        const int points = o.value(QLatin1String("review_points")).toInt(0);
        QString url = normalizeUrl(o.value(QLatin1String("url")).toString());
        if (!isValidProductUrl(url))
            continue;
        QString shortName = name;
        if (shortName.length() > 80)
            shortName = shortName.left(77) + QStringLiteral("...");
        out.append(Product(index++, shortName, price, points, url));
    }
    return out;
}


QVector<Product> OzonScraper::computeTop50(const QVector<Product>& all) const
{
    QVector<Product> filtered;
    for (const Product& p : all) {
        const int pts = p.reviewPoints();
        if (minPoints_ >= 0 && pts < minPoints_)
            continue;
        if (maxPoints_ >= 0 && pts > maxPoints_)
            continue;
        filtered.append(p);
    }
    std::sort(filtered.begin(), filtered.end(), [](const Product& a, const Product& b) {
        return a.pointsToPriceRatio() > b.pointsToPriceRatio();
    });
    const int n = qMin(50, filtered.size());
    QVector<Product> top;
    top.reserve(n);
    for (int i = 0; i < n; ++i) {
        const Product& orig = filtered[i];
        top.append(Product(i + 1, orig.name(), orig.price(), orig.reviewPoints(), orig.url()));
    }
    return top;
}


void OzonScraper::finishWithError(const QString& message)
{
    running_ = false;
    scrollTimer_->stop();
    if (page_) {
        page_->deleteLater();
        page_ = nullptr;
    }
    emit finishedWithError(message);
}


void OzonScraper::finishWithSuccess()
{
    running_ = false;
    scrollTimer_->stop();

    const QVector<Product> top = computeTop50(allProducts_);
    const int total = allProducts_.size();
    const QString elapsed = formatElapsed(elapsedTimer_.elapsed());

    if (page_) {
        page_->deleteLater();
        page_ = nullptr;
    }

    if (total == 0) {
        emit finishedWithError(QStringLiteral("Товары не найдены. ") + elapsed);
        return;
    }

    emit topProductsUpdated(top, total);
    emit finishedSuccessfully(total, elapsed);
}


QString OzonScraper::formatElapsed(qint64 ms) const
{
    const double sec = ms / 1000.0;
    if (sec < 60)
        return QStringLiteral("Затрачено %1 сек").arg(sec, 0, 'f', 1);
    const int m = static_cast<int>(sec / 60);
    const double s = sec - m * 60;
    if (s < 0.1)
        return QStringLiteral("Затрачено %1 мин").arg(m);
    return QStringLiteral("Затрачено %1 мин %2 сек").arg(m).arg(s, 0, 'f', 1);
}
