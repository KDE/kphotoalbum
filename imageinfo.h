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

class ImageInfo :public QObject {

public:
    ImageInfo();
    ImageInfo( const QString& fileName, QDomElement elm );

    QString fileName();

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setDate( int year,  int month,  int day );
    int year() const;
    int month() const;
    int day() const;

    void setTime( int hour,  int minute );
    int hour() const;
    int minute() const;

    void setQuality( int );
    int quality() const;

    void setOption( const QString& key,  const QStringList& value );
    QStringList optionValue( const QString& key ) const;

    QPixmap pixmap( int width,  int height );
    QDomElement save( QDomDocument& doc );

private:
     // This string is accessed on several threads, so we need to make it a deep copy!
    QDeepCopy<QString> _fileName;
    QString _label;
    QString _description;
    int _year,  _month,  _day, _hour, _minute;
    int _quality;
    QMap<QString, QStringList> _options;
};

typedef QPtrList<ImageInfo> ImageInfoList;

#endif /* IMAGEINFO_H */

