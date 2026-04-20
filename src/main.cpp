#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ozon_scraper/ozonradarscraper.h"
#include "product.h"
#include "productmodel.h"
#include "settingsservice.h"
#include "shortcutlayoutfix.h"


int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ozon_radar");
    QCoreApplication::setApplicationName("ozon_radar");

    ShortcutLayoutFixFilter shortcutLayoutFix(&app);
    app.installEventFilter(&shortcutLayoutFix);

    OzonRadarScraper scraper;
    ProductModel productModel;
    SettingsService settings;

    QObject::connect(&scraper, 
                     &OzonRadarScraper::topProductsUpdated,
                     [&productModel](const QVector<Product>& products, int totalCount) {
                            productModel.setProducts(products, totalCount);
                        });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("scraper", &scraper);
    engine.rootContext()->setContextProperty("productModel", &productModel);
    engine.rootContext()->setContextProperty("settings", &settings);

    engine.load(QUrl("qrc:/qml/main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
