#ifndef IMAGECLIENT_H
#define IMAGECLIENT_H
class QPixmap;
class QString;

class ImageClient {
public:
    virtual ~ImageClient();
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QPixmap& ) = 0;
};

#endif /* IMAGECLIENT_H */

