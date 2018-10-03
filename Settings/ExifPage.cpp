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
#include "ExifPage.h"
#include "SettingsData.h"
#include <QTextCodec>
#include <KLocalizedString>
#include <KComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Exif/Info.h"
#include "Exif/TreeView.h"

Settings::ExifPage::ExifPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* vlay = new QVBoxLayout( this );
    QHBoxLayout* hlay1 = new QHBoxLayout();
    QHBoxLayout* hlay2 = new QHBoxLayout();
    vlay->addLayout( hlay1 );
    vlay->addLayout( hlay2 );

    m_exifForViewer = new Exif::TreeView( i18n( "Exif/IPTC info to show in the viewer" ), this );
    hlay1->addWidget( m_exifForViewer );

    m_exifForDialog = new Exif::TreeView( i18n("Exif/IPTC info to show in the Exif dialog"), this );
    hlay1->addWidget( m_exifForDialog );

    QLabel* iptcCharsetLabel = new QLabel( i18n("Character set for image metadata:"), this );
    m_iptcCharset = new KComboBox( this );
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it )
        charsets << QString::fromLatin1(*it);
    m_iptcCharset->insertItems( m_iptcCharset->count(), charsets );

    hlay2->addStretch( 1 );
    hlay2->addWidget( iptcCharsetLabel );
    hlay2->addWidget( m_iptcCharset );

}

void Settings::ExifPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setExifForViewer( m_exifForViewer->selected() ) ;
    opt->setExifForDialog( m_exifForDialog->selected() ) ;
    opt->setIptcCharset( m_iptcCharset->currentText() );

}

void Settings::ExifPage::loadSettings( Settings::SettingsData* opt )
{
    m_exifForViewer->reload();
    m_exifForDialog->reload();
    m_exifForViewer->setSelectedExif( Settings::SettingsData::instance()->exifForViewer() );
    m_exifForDialog->setSelectedExif( Settings::SettingsData::instance()->exifForDialog() );
    m_iptcCharset->setCurrentIndex( qMax( 0, QTextCodec::availableCodecs().indexOf( opt->iptcCharset().toLatin1() ) ) );
}
// vi:expandtab:tabstop=4 shiftwidth=4:
