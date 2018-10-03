/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextCodec>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

#include "DB/ImageDB.h"
#include "Exif/InfoDialog.h"
#include "Exif/Info.h"
#include "ImageManager/AsyncLoader.h"
#include "ImageManager/ImageRequest.h"
#include "Settings/SettingsData.h"
#include "Grid.h"

using Utilities::StringSet;

Exif::InfoDialog::InfoDialog(const DB::FileName& fileName, QWidget* parent) : QDialog(parent)
{
    setWindowTitle( i18nc("@title:window", "Exif Information") );

    setAttribute(Qt::WA_DeleteOnClose);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Close)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QWidget* top = new QWidget(this);
    QVBoxLayout* vlay = new QVBoxLayout( top );
    setLayout(vlay);
    vlay->addWidget(top);

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

    QLabel* searchLabel = new QLabel( i18n( "Exif label search: "), top );
    hlay->addWidget( searchLabel );
    m_searchBox = new QLineEdit( top );
    hlay->addWidget( m_searchBox );
    hlay->addStretch( 1 );

    QLabel* iptcLabel = new QLabel( i18n("IPTC character set:"), top );
    m_iptcCharset = new QComboBox( top );
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it )
        charsets << QLatin1String(*it);
    m_iptcCharset->insertItems( 0, charsets );
    m_iptcCharset->setCurrentIndex( qMax( 0, QTextCodec::availableCodecs().indexOf( Settings::SettingsData::instance()->iptcCharset().toLatin1() ) ) );
    hlay->addWidget( iptcLabel );
    hlay->addWidget( m_iptcCharset );

    connect( m_searchBox, SIGNAL(textChanged(QString)), m_grid, SLOT(updateSearchString(QString)) );
    connect( m_iptcCharset, SIGNAL(activated(QString)), m_grid, SLOT(setupUI(QString)) );
    setImage(fileName);

    vlay->addWidget(buttonBox);
}

QSize Exif::InfoDialog::sizeHint() const
{
    return QSize( 800, 400 );
}

void Exif::InfoDialog::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    if ( request->loadedOK() )
        m_pix->setPixmap( QPixmap::fromImage(image) );
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

// vi:expandtab:tabstop=4 shiftwidth=4:
