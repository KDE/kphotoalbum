#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <imageinfo.h>
#include "imageclient.h"
class KZip;
class QProgressDialog;

class Export :public ImageClient {

public:
    static void imageExport( const ImageInfoList& list );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );

protected:
    QCString createIndexXML( const ImageInfoList& );
    void generateThumbnails( const ImageInfoList& list );
    void copyImages( const ImageInfoList& list );

private:
    Export(  const ImageInfoList& list );
    int _filesRemaining;
    int _steps;
    QProgressDialog* _progressDialog;
    bool _ok;
    KZip* _zip;
    QMap<QString,QString> _map;
};


#endif /* IMPORTEXPORT_H */

