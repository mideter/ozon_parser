#pragma once

#include <QString>
#include <QStringList>


class OzonRadarScraperSettings
{
public:
    OzonRadarScraperSettings(const QString& urls, int minPoints, int maxPoints);

    QStringList urls() const;
    int maxPoints() const;
    int minPoints() const;

private:
    static QStringList parseUrls(const QString& text);

    QStringList urls_;
    int minPoints_ = -1;
    int maxPoints_ = -1;
};
