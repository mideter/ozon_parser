#include "ozon_scraper/fetchscriptpathresolver.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>


QString FetchScriptPathResolver::resolve()
{
    const QByteArray env = qgetenv("OZON_FETCH_SCRIPT");
    if (!env.isEmpty())
        return QString::fromLocal8Bit(env);

    const QString appDir = QCoreApplication::applicationDirPath();
    QString scriptPath = QDir(appDir).filePath("../scripts/ozon_fetch.py");

    if (QFileInfo::exists(scriptPath))
        return QDir::cleanPath(scriptPath);

    scriptPath = QDir::currentPath() + "/scripts/ozon_fetch.py";
    if (QFileInfo::exists(scriptPath))
        return QDir::cleanPath(scriptPath);

    return QDir::cleanPath(QDir(appDir).filePath("../scripts/ozon_fetch.py"));
}
