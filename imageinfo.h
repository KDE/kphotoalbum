#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qpixmap.h>
#include <qdom.h>
#include <qobject.h>
#include <qdeepcopy.h>
#ifndef IMAGEINFO_H
#define IMAGEINFO_H

class ImageDate {
public:
    ImageDate();
    ImageDate( int day, int month, int year );
    int year() const;
    int month() const;
    int day() const;
    void setYear( int );
    void setMonth( int );
    void setDay( int );
    bool operator<=( ImageDate& other );
    bool isNull() const;
    operator QString()  {
        return QString("%1/%2-%3").arg(_day).arg(_month).arg(_year);
    }
private:
    int _year, _month, _day;
};


class ImageInfo {

public:
    ImageInfo();
    ImageInfo( const QString& fileName, QDomElement elm );

    QString fileName();

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setStartDate( const ImageDate& );
    void setEndDate( const ImageDate& );
    ImageDate& startDate();
    ImageDate& endDate();

    void setQuality( int );
    int quality() const;

    void rotate( int degrees );
    int angle() const;

    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QStringList& value );
    QStringList optionValue( const QString& key ) const;

    QDomElement save( QDomDocument& doc );

private:
     // This string is accessed on several threads, so we need to make it a deep copy!
    QString _fileName;
    QString _label;
    QString _description;
    ImageDate _startDate, _endDate;
    int _quality;
    QMap<QString, QStringList> _options;
    int _angle;
};

typedef QPtrList<ImageInfo> ImageInfoList;
typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFO_H */

