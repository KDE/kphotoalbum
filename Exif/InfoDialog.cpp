/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "Exif/InfoDialog.h"
#include <KComboBox>
#include <klocale.h>
#include "Exif/Info.h"
#include <qlayout.h>
#include <qlabel.h>
#include <QTextCodec>
#include "ImageManager/AsyncLoader.h"
#include "ImageManager/ImageRequest.h"
#include "DB/ImageDB.h"
#include "Settings/SettingsData.h"
#include "Grid.h"

using Utilities::StringSet;

Exif::InfoDialog::InfoDialog( const DB::FileName& fileName, QWidget* parent )
    :KDialog( parent )
{
    setWindowTitle( i18n("EXIF Information") );
    setButtons( Close );
    setAttribute( Qt::WA_DeleteOnClose );

    QWidget* top = new QWidget;
    setMainWidget( top );
    QVBoxLayout* vlay = new QVBoxLayout( top );

    // -------------------------------------------------- File name and pixmap
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    m_fileNameLabel = new QLabel( top );
    QFont fnt = font();
    fnt.setPointSize( (int) (fnt.pointSize() * 1.2) );
    fnt.setWeight( QFont::Bold );
    m_fileNameLabel->setFont( fnt );
    m_fileNameLabel->setAlignment( Qt::AlignCenter );
    hlay->addWidget( m_fileNameLabel, 1 );

    m_pix = new QLabel( top );
    hlay->addWidget( m_pix );

    // -------------------------------------------------- Exif Grid
    m_grid = new Exif::Grid( top );
    vlay->addWidget( m_grid );

    // -------------------------------------------------- Current Search
    hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    m_fileNameLabel = new QLabel( i18n( "Current EXIF Label Search: "), top );
    hlay->addWidget( m_fileNameLabel );

    m_searchLabel = new QLabel( top );
    QPalette pal = m_searchLabel->palette();
    pal.setColor( QPalette::Foreground, Qt::red );
    m_searchLabel->setPalette(pal);
    fnt = font();
    fnt.setWeight( QFont::Bold );
    m_searchLabel->setFont( fnt );

    hlay->addWidget( m_searchLabel );
    hlay->addStretch( 1 );

    QLabel* iptcLabel = new QLabel( i18n("IPTC character set:"), top );
    m_iptcCharset = new KComboBox( top );
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it )
        charsets << QLatin1String(*it);
    m_iptcCharset->insertItems( 0, charsets );
    m_iptcCharset->setCurrentIndex( qMax( 0, QTextCodec::availableCodecs().indexOf( Settings::SettingsData::instance()->iptcCharset().toAscii() ) ) );
    hlay->addWidget( iptcLabel );
    hlay->addWidget( m_iptcCharset );

    connect( m_grid, SIGNAL( searchStringChanged( const QString& ) ), this, SLOT( updateSearchString( const QString& ) ) );
    connect( m_iptcCharset, SIGNAL( activated( const QString& ) ), m_grid, SLOT( setupUI( const QString& ) ) );
    setImage(fileName);
    updateSearchString( QString() );
}

void Exif::InfoDialog::updateSearchString( const QString& txt )
{
    if( txt.isEmpty() )
        m_searchLabel->setText( i18n("<No Search>") );
    else
        m_searchLabel->setText( txt );
}

QSize Exif::InfoDialog::sizeHint() const
{
    return QSize( 800, 400 );
}

void Exif::InfoDialog::pixmapLoaded( const DB::FileName& , const QSize& , const QSize& , int , const QImage& img, const bool loadedOK)
{
    if ( loadedOK )
      m_pix->setPixmap( QPixmap::fromImage(img) );
}

void Exif::InfoDialog::setImage(const DB::FileName& fileName )
{
    m_fileNameLabel->setText( fileName.relative() );
    m_grid->setFileName( fileName );

    ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( 128, 128 ), fileName.info()->angle(), this );
    request->setPriority( ImageManager::Viewer );
    ImageManager::AsyncLoader::instance()->load( request );
}

void Exif::InfoDialog::enterEvent(QEvent *)
{
    m_grid->setFocus();
}

#include "InfoDialog.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
