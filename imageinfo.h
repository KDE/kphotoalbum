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

    void setYearFrom( int year );
    void setMonthFrom( int month );
    void setDayFrom( int day );
    int yearFrom() const;
    int monthFrom() const;
    int dayFrom() const;

    void setYearTo( int year );
    void setMonthTo( int month );
    void setDayTo( int day );
    int yearTo() const;
    int monthTo() const;
    int dayTo() const;

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
    QDeepCopy<QString> _fileName;
    QString _label;
    QString _description;
    int _yearFrom,  _monthFrom,  _dayFrom,  _yearTo,  _monthTo,  _dayTo;
    int _quality;
    QMap<QString, QStringList> _options;
    int _angle;
};

typedef QPtrList<ImageInfo> ImageInfoList;
typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFO_H */

