#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "imagesearchinfo.h"
#include "imagedate.h"

class ImageSearchInfo {
public:
    ImageSearchInfo();
    ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                     const QString& persons, const QString& locations, const QString& keywords,
                     const QString& label, const QString& description );

    ImageDate startDate() const;
    ImageDate endDate() const;

    QString persons() const;
    QString locations() const;
    QString keywords() const;

    QString label() const;
    QString description() const;

private:
    ImageDate _startDate;
    ImageDate _endDate;
    QString _persons;
    QString _locations;
    QString _keywords;
    QString _label;
    QString _description;
};


#endif /* IMAGESEARCHINFO_H */

