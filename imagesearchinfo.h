#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "imagesearchinfo.h"
#include "imagedate.h"

class ImageSearchInfo {
public:
    ImageSearchInfo();
    ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate, int startQuality, int endQuality,
                     const QString& persons, const QString& locations, const QString& keywords, const QString& items,
                     const QString& label, const QString& description );

    ImageDate startDate() const;
    ImageDate endDate() const;

    int startQuality() const;
    int endQuality() const;

    QString persons() const;
    QString locations() const;
    QString keywords() const;
    QString items() const;

    QString label() const;
    QString description() const;

private:
    ImageDate _startDate;
    ImageDate _endDate;
    int _startQuality;
    int _endQuality;
    QString _persons;
    QString _locations;
    QString _keywords;
    QString _items;
    QString _label;
    QString _description;
};


#endif /* IMAGESEARCHINFO_H */

