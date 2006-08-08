/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPHOTOALBUM_PLUGININTERFACE_H
#define KPHOTOALBUM_PLUGININTERFACE_H

#ifdef HASKIPI
#include <libkipi/interface.h>
#include <qvaluelist.h>
#include <libkipi/imagecollection.h>
#include <libkipi/imageinfo.h>
#include <kurl.h>
#include <kdemacros.h>

namespace Plugins
{

class KDE_EXPORT Interface :public KIPI::Interface
{
    Q_OBJECT

public:
    Interface( QObject *parent, const char *name=0);
    virtual KIPI::ImageCollection currentAlbum();
    virtual KIPI::ImageCollection currentSelection();
    virtual QValueList<KIPI::ImageCollection> allAlbums();
    virtual KIPI::ImageInfo info( const KURL& );
    virtual bool addImage( const KURL&, QString& errmsg );
    virtual void delImage( const KURL& );
    virtual void refreshImages( const KURL::List& urls );
    virtual int features() const;

public slots:
    void slotSelectionChanged( bool );
    void pathChanged( const QString& path );

signals:
    void imagesChanged( const KURL::List& );
};

}

#endif

#endif /* PLUGININTERFACE_H */

