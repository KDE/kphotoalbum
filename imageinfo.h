#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qpixmap.h>
#include <qdom.h>
#include <qobject.h>
#include <qdeepcopy.h>
#include "imagedate.h"
#include "drawlist.h"

class ImageInfo {

public:
    ImageInfo();
    ImageInfo( const QString& fileName );
    ImageInfo( const QString& fileName, QDomElement elm );
    void setVisible( bool b );
    bool visible() const;

    QString fileName( bool relative );

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setStartDate( const ImageDate& );
    void setEndDate( const ImageDate& );
    ImageDate& startDate();
    ImageDate& endDate();

    void rotate( int degrees );
    int angle() const;

    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QStringList& value );
    void removeOption( const QString& key, const QString& value );
    bool hasOption( const QString& key,  const QString& value );
    QStringList optionValue( const QString& key ) const;
    void renameOption( const QString& key, const QString& oldValue, const QString& newValue );
    void renameOptionGroup( const QString& oldName, const QString& newName );

    QDomElement save( QDomDocument& doc );
    bool operator!=( const ImageInfo& other );
    bool operator==( const ImageInfo& other );

    DrawList drawList() const;
    void setDrawList( const DrawList& );

    bool imageOnDisk() const { return _imageOnDisk; }

private:
    // This string is accessed on several threads, so we need to make it a deep copy!
    // PENDING(blackie) Is the above comment true?
    QString _fileName;
    QString _label;
    QString _description;
    ImageDate _startDate, _endDate;
    QMap<QString, QStringList> _options;
    int _angle;
    bool _visible;
    DrawList _drawList;
    bool _imageOnDisk; // true if the image is available on disk
};

typedef QPtrList<ImageInfo> ImageInfoList;
typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFO_H */

