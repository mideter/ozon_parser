#include "ozon_scraper/ozonradarscrapersettings.h"

#include <QUrl>
#include <stdexcept>


OzonRadarScraperSettings::OzonRadarScraperSettings(
            const QString& urls,
            int minPoints,
            int maxPoints)
    : urls_(parseUrls(urls))
    , minPoints_(minPoints)
    , maxPoints_(maxPoints)
{}


QStringList OzonRadarScraperSettings::urls() const
{
    return urls_;
}


int OzonRadarScraperSettings::maxPoints() const
{
    return maxPoints_;
}


int OzonRadarScraperSettings::minPoints() const
{
    return minPoints_;
}


QStringList OzonRadarScraperSettings::parseUrls(const QString& text)
{
    QStringList out;
    const QStringList rawLines = text.split(QChar('\n'));

    for (QString line : rawLines) {
        line = line.trimmed();

        if (line.isEmpty())
            continue;

        if (!line.startsWith("http://", Qt::CaseInsensitive)
            && !line.startsWith("https://", Qt::CaseInsensitive)) {
            line = "https://" + line;
        }

        const QUrl url = QUrl::fromUserInput(line);
        if (url.isValid() && !url.scheme().isEmpty())
            out.append(url.toString());
    }

    if (out.isEmpty())
        throw std::runtime_error("Некорректные URL. Укажите по одной ссылке в строке.");

    return out;
}
