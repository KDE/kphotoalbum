#ifndef KIMDABA_PLUGININTERFACE_H
#define KIMDABA_PLUGININTERFACE_H

#include <libkipi/interface.h>
#include <qvaluelist.h>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <kurl.h>

class PluginInterface :public KIPI::Interface
{
    Q_OBJECT

public:
    PluginInterface( QObject *parent, const char *name=0);
    virtual KIPI::ImageCollection currentAlbum();
    virtual KIPI::ImageCollection currentSelection();
    virtual QValueList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KURL& );
    virtual bool addImage( const KURL&, QString& errmsg );
    virtual void refreshImages( const KURL::List& urls );
    virtual int features() const;

protected:
    QString askForCategory();

signals:
    void imagesChanged( const KURL::List& );
};

#endif /* PLUGININTERFACE_H */

