/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPHOTOALBUM_PLUGININTERFACE_H
#define KPHOTOALBUM_PLUGININTERFACE_H

#include <config-kpa-kipi.h>
#include <libkipi/interface.h>
#include <QList>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <libkipi/imagecollectionselector.h>
#include <libkipi/uploadwidget.h>
#include <kurl.h>
#include <kdemacros.h>

namespace Browser {
class BreadcrumbList;
}

namespace Plugins
{

class KDE_EXPORT Interface :public KIPI::Interface
{
    Q_OBJECT

public:
    Interface( QObject *parent, const char *name=0);
    virtual KIPI::ImageCollection currentAlbum();
    virtual KIPI::ImageCollection currentSelection();
    virtual QList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KUrl& );
    virtual bool addImage( const KUrl&, QString& errmsg );
    virtual void delImage( const KUrl& );
    virtual void refreshImages( const KUrl::List& urls );
    virtual int features() const;
    virtual KIPI::ImageCollectionSelector* imageCollectionSelector(QWidget *parent);
    virtual KIPI::UploadWidget* uploadWidget(QWidget *parent);

public slots:
    void slotSelectionChanged( bool );
    void pathChanged( const Browser::BreadcrumbList& path );

signals:
    void imagesChanged( const KUrl::List& );
};

}


#endif /* PLUGININTERFACE_H */

