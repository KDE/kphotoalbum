#ifndef IMAGEREQUEST_H
#define IMAGEREQUEST_H
#include <qstring.h>
#include "imageclient.h"
#include <qdeepcopy.h>
#include <qsize.h>
#include <qmutex.h>

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This class is shared among the image loader thead and the GUI tread, if
// you don't know the implication of this stay out of this class!
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

class ImageRequest {
public:
    virtual ~ImageRequest() {}
    ImageRequest();
    ImageRequest( const QString& fileName, const QSize& size, int angle, ImageClient* client);

    bool isNull() const;
    QString fileName() const;
    int width() const;
    int height() const;
    int angle() const;

    void setCache( bool b = true );
    bool cache() const;
    ImageClient* client() const;

    QSize fullSize() const;
    void setFullSize( const QSize& );
    void setLoadedOK( bool ok );
    bool loadedOK() const;

    void setPriority( bool b = true );
    bool priority() const;

    bool operator<( const ImageRequest& other ) const;
    bool operator==( const ImageRequest& other ) const;

    virtual bool stillNeeded() const;

private:
    bool _null;
    mutable QDeepCopy<QString> _fileName;
    mutable QMutex _fileNameLock;

    int _width;
    int _height;
    bool _cache;
    ImageClient* _client;
    int _angle;
    QSize _fullSize;
    bool _priority;
    bool _loadedOK;
};



#endif /* IMAGEREQUEST_H */

