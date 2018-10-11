/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ToolTip.h"
#include "Settings/SettingsData.h"
#include "DB/ImageDB.h"
#include "ImageManager/ImageRequest.h"
#include "ImageManager/AsyncLoader.h"
#include <QTemporaryFile>
#include "Utilities/DescriptionUtil.h"

namespace Utilities {

ToolTip::ToolTip(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent, f), m_tmpFileForThumbnailView(nullptr)
{
    setAlignment( Qt::AlignLeft | Qt::AlignTop );
    setLineWidth(1);
    setMargin(1);

    setWindowOpacity(0.8);
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(QPalette::Background, QColor(0,0,0,170)); // r,g,b,A
    p.setColor(QPalette::WindowText, Qt::white );
    setPalette(p);
}

void ToolTip::requestImage( const DB::FileName& fileName )
{
    int size = Settings::SettingsData::instance()->previewSize();
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName );
    if ( size != 0 ) {
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( size, size ), info->angle(), this );
        request->setPriority( ImageManager::Viewer );
        ImageManager::AsyncLoader::instance()->load( request );
    }
    else
        renderToolTip();
}

void ToolTip::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    const DB::FileName fileName = request->databaseFileName();

    delete m_tmpFileForThumbnailView;
    m_tmpFileForThumbnailView = new QTemporaryFile(this);
    m_tmpFileForThumbnailView->open();

    image.save(m_tmpFileForThumbnailView, "PNG" );
    if ( fileName == m_currentFileName )
        renderToolTip();
}

void ToolTip::requestToolTip(const DB::FileName &fileName)
{
    if ( fileName.isNull() || fileName == m_currentFileName)
        return;
    m_currentFileName = fileName;
    requestImage( fileName );
}


void ToolTip::renderToolTip()
{
    const int size = Settings::SettingsData::instance()->previewSize();
    if ( size != 0 ) {
        setText( QString::fromLatin1("<table cols=\"2\" cellpadding=\"10\"><tr><td><img src=\"%1\"></td><td>%2</td></tr>")
                 .arg(m_tmpFileForThumbnailView->fileName()).
                 arg(Utilities::createInfoText( DB::ImageDB::instance()->info( m_currentFileName ), nullptr ) ) );
    }
    else
        setText( QString::fromLatin1("<p>%1</p>").arg( Utilities::createInfoText( DB::ImageDB::instance()->info( m_currentFileName ), nullptr ) ) );

    setWordWrap( true );

    resize( sizeHint() );
//    m_view->setFocus();
    show();
    placeWindow();
}


} // namespace Utilities
// vi:expandtab:tabstop=4 shiftwidth=4:
