#ifndef IMAGESEARCHINFO_H
#define IMAGESEARCHINFO_H
#include "imagesearchinfo.h"
#include "imagedate.h"
#include "imageinfo.h"

class ImageSearchInfo {
public:
    ImageSearchInfo();
    ImageSearchInfo( const ImageDate& startDate, const ImageDate& endDate,
                     const QString& label, const QString& description );

    void setStartDate( const ImageDate& );
    void setEndDate( const ImageDate& );
    ImageDate startDate() const;
    ImageDate endDate() const;

    QString option( const QString& name ) const;
    void setOption( const QString& name, const QString& value );

    QString label() const;
    QString description() const;

    bool isNull();
    bool match( ImageInfo* );

    void addAnd( const QString& group, const QString& value );

protected:
    bool stringMatch( const QString& key, ImageInfo* info );

private:
    ImageDate _startDate;
    ImageDate _endDate;
    QMap<QString, QString> _options;
    QString _label;
    QString _description;
    bool _isNull;
};


#endif /* IMAGESEARCHINFO_H */

