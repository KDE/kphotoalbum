#ifndef IMAGECLIENT_H
#define IMAGECLIENT_H
class QImage;
class QString;

class ImageClient {
public:
    virtual ~ImageClient();
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& ) = 0;
};

#endif /* IMAGECLIENT_H */

