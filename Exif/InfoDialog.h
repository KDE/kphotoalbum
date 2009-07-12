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
#ifndef EXIFDIALOG_H
#define EXIFDIALOG_H
#include <KDialog>
#include <q3gridview.h>
#include "Utilities/Set.h"
#include "ImageManager/ImageClient.h"
class QComboBox;
class QLabel;
class QKeyEvent;
class QResizeEvent;

namespace DB { class ResultId; }

namespace Exif
{
using Utilities::StringSet;

class InfoDialog : public KDialog, public ImageManager::ImageClient {
    Q_OBJECT

public:
    InfoDialog( const DB::ResultId& id, QWidget* parent );

    virtual QSize sizeHint() const;

    // ImageManager::ImageClient interface.
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, const bool loadedOK);

protected slots:
    void updateSearchString( const QString& );

private:
    QLabel* _searchLabel;
    QLabel* _pix;
    QComboBox* _iptcCharset;
};

class Grid :public Q3GridView
{
    Q_OBJECT

public:
    Grid( const QString& fileName, QWidget* parent, const char* name = 0 );

signals:
    QString searchStringChanged( const QString& text );

protected:
    virtual void paintCell ( QPainter * p, int row, int col );
    virtual void resizeEvent( QResizeEvent* );
    virtual void keyPressEvent( QKeyEvent* );

    StringSet exifGroups( const QMap<QString, QStringList>& exifInfo );
    QMap<QString,QStringList> itemsForGroup( const QString& group, const QMap<QString, QStringList>& exifInfo );
    QString groupName( const QString& exifName );
    QString exifNameNoGroup( const QString& fullName );
    void calculateMaxKeyWidth( const QMap<QString, QStringList>& exifInfo );

protected slots:
    void updateGrid();
    void slotCharsetChange( const QString& charset );

private:
    QMap<int, QPair<QString,QStringList> > _texts;
    QSet<int> _headers;
    int _maxKeyWidth;
    QString _search;
    QString _fileName;
};

}

#endif /* EXIFDIALOG_H */

