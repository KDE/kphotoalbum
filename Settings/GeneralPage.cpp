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
#include "GeneralPage.h"
#include <DB/ImageDB.h>
#include <DB/Category.h>
#include <KComboBox>
#include <KLocalizedString>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include "DB/CategoryCollection.h"
#include "SettingsData.h"
#include "MainWindow/Window.h"
#include <QGroupBox>
#include <QTextEdit>

Settings::GeneralPage::GeneralPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    QGroupBox* box = new QGroupBox( i18n( "Loading New Images" ), this );
    lay1->addWidget( box );

    QGridLayout* lay = new QGridLayout( box );
    lay->setSpacing( 6 );
    int row = 0;

    // Thrust time stamps
    QLabel* timeStampLabel = new QLabel( i18n("Trust image dates:"), box );
    m_trustTimeStamps = new KComboBox( box );
    m_trustTimeStamps->addItems( QStringList() << i18nc("As in 'always trust image dates'","Always")
            << i18nc("As in 'ask whether to trust image dates'","Ask")
            << i18nc("As in 'never trust image dates'","Never") );
    timeStampLabel->setBuddy( m_trustTimeStamps );
    lay->addWidget( timeStampLabel, row, 0 );
    lay->addWidget( m_trustTimeStamps, row, 1, 1, 3 );

    // Do Exif rotate
    row++;
    m_useEXIFRotate = new QCheckBox( i18n( "Use Exif orientation information" ), box );
    lay->addWidget( m_useEXIFRotate, row, 0, 1, 4 );

    // Use Exif description
    row++;
    m_useEXIFComments = new QCheckBox( i18n( "Use Exif description" ), box );
    lay->addWidget( m_useEXIFComments, row, 0, 1, 4 );
    connect(m_useEXIFComments, &QCheckBox::stateChanged, this, &GeneralPage::useEXIFCommentsChanged);

    m_stripEXIFComments = new QCheckBox(i18n("Strip out camera generated default descriptions"), box);
    connect(m_stripEXIFComments, &QCheckBox::stateChanged, this, &GeneralPage::stripEXIFCommentsChanged);
    lay->addWidget(m_stripEXIFComments, row, 1, 1, 4);

    row++;
    m_commentsToStrip = new QTextEdit();
    m_commentsToStrip->setMaximumHeight(60);
    m_commentsToStrip->setEnabled(false);
    lay->addWidget(m_commentsToStrip, row, 1, 1, 4);

    // Use embedded thumbnail
    row++;
    m_useRawThumbnail = new QCheckBox( i18n("Use the embedded thumbnail in RAW file or halfsized RAW"), box );
    lay->addWidget( m_useRawThumbnail, row, 0 );

    row++;
    QLabel* label = new QLabel( i18n("Required size for the thumbnail:"), box );
    m_useRawThumbnailWidth = new QSpinBox( box );
    m_useRawThumbnailWidth->setRange( 100, 5000 );
    m_useRawThumbnailWidth->setSingleStep( 64 );
    lay->addWidget( label, row, 0 );
    lay->addWidget( m_useRawThumbnailWidth, row, 1 );

    label = new QLabel( QString::fromLatin1("x"), box );
    m_useRawThumbnailHeight = new QSpinBox( box );
    m_useRawThumbnailHeight->setRange( 100, 5000 );
    m_useRawThumbnailHeight->setSingleStep( 64 );
    lay->addWidget( label, row, 2 );
    lay->addWidget( m_useRawThumbnailHeight, row, 3 );

    box = new QGroupBox( i18n( "Histogram" ), this );
    lay1->addWidget( box );

    lay = new QGridLayout( box );
    lay->setSpacing( 6 );
    row = 0;

    m_showHistogram = new QCheckBox( i18n("Show histogram:"), box);
    lay->addWidget( m_showHistogram, row, 0 );
    connect(m_showHistogram, &QCheckBox::stateChanged, this, &GeneralPage::showHistogramChanged);

    row++;
    label = new QLabel( i18n("Size of histogram columns in date bar:"), box );
    m_barWidth = new QSpinBox;
    m_barWidth->setRange( 1, 100 );
    m_barWidth->setSingleStep( 1 );
    lay->addWidget( label, row, 0 );
    lay->addWidget( m_barWidth, row, 1 );

    label = new QLabel( QString::fromLatin1("x"), box );
    m_barHeight = new QSpinBox;
    m_barHeight->setRange( 15, 100 );
    lay->addWidget( label, row, 2 );
    lay->addWidget( m_barHeight, row, 3 );

    box = new QGroupBox( i18n( "Miscellaneous" ), this );
    lay1->addWidget( box );

    lay = new QGridLayout( box );
    lay->setSpacing( 6 );
    row = 0;

    // Show splash screen
    m_showSplashScreen = new QCheckBox( i18n("Show splash screen"), box );
    lay->addWidget( m_showSplashScreen, row, 0 );

    // Album Category
    row++;
    QLabel* albumCategoryLabel = new QLabel( i18n("Category for virtual albums:" ), box );
    m_albumCategory = new QComboBox;
    lay->addWidget( albumCategoryLabel, row, 0 );
    lay->addWidget( m_albumCategory, row, 1 );

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    Q_FOREACH( const DB::CategoryPtr category, categories ) {
       m_albumCategory->addItem( category->name() );
    }

    m_listenForAndroidDevicesOnStartup = new QCheckBox(i18n("Listen for Android devices on startup"));
    lay->addWidget(m_listenForAndroidDevicesOnStartup);

    lay1->addStretch( 1 );

    // Whats This
    QString txt;

    txt = i18n( "<p>KPhotoAlbum will try to read the image date from Exif information in the image. "
                "If that fails it will try to get the date from the file's time stamp.</p>"
                "<p>However, this information will be wrong if the image was scanned in (you want the date the image "
                "was taken, not the date of the scan).</p>"
                "<p>If you only scan images, in contrast to sometimes using "
                "a digital camera, you should reply <b>no</b>. If you never scan images, you should reply <b>yes</b>, "
                "otherwise reply <b>ask</b>. This will allow you to decide whether the images are from "
                "the scanner or the camera, from session to session.</p>" );
    timeStampLabel->setWhatsThis( txt );
    m_trustTimeStamps->setWhatsThis( txt );

    txt = i18n( "<p>JPEG images may contain information about rotation. "
                "If you have a reason for not using this information to get a default rotation of "
                "your images, uncheck this check box.</p>"
                "<p>Note: Your digital camera may not write this information into the images at all.</p>" );
    m_useEXIFRotate->setWhatsThis( txt );

    txt = i18n( "<p>JPEG images may contain a description. "
               "Check this checkbox to specify if you want to use this as a "
               "default description for your images.</p>" );
    m_useEXIFComments->setWhatsThis( txt );

    txt = i18n("<p>KPhotoAlbum shares plugins with other imaging applications, some of which have the concept of albums. "
               "KPhotoAlbum does not have this concept; nevertheless, for certain plugins to function, KPhotoAlbum behaves "
               "to the plugin system as if it did.</p>"
               "<p>KPhotoAlbum does this by defining the current album to be the current view - that is, all the images the "
               "browser offers to display.</p>"
               "<p>In addition to the current album, KPhotoAlbum must also be able to give a list of all albums; "
               "the list of all albums is defined in the following way:"
               "<ul><li>When KPhotoAlbum's browser displays the content of a category, say all People, then each item in this category "
               "will look like an album to the plugin.</li>"
               "<li>Otherwise, the category you specify using this option will be used; e.g. if you specify People "
               "with this option, then KPhotoAlbum will act as if you had just chosen to display people and then invoke "
               "the plugin which needs to know about all albums.</li></ul></p>"
               "<p>Most users would probably want to specify Events here.</p>");
    albumCategoryLabel->setWhatsThis( txt );
    m_albumCategory->setWhatsThis( txt );

    txt = i18n( "Show the KPhotoAlbum splash screen on start up" );
    m_showSplashScreen->setWhatsThis( txt );

    txt = i18n("<p>KPhotoAlbum is capable of showing your images on android devices. KPhotoAlbum will automatically pair with the app from "
               "android. This, however, requires that KPhotoAlbum on your desktop is listening for multicast messages. "
               "Checking this checkbox will make KPhotoAlbum do so automatically on start up. "
               "Alternatively, you can click the connection icon in the status bar to start listening.");
    m_listenForAndroidDevicesOnStartup->setWhatsThis(txt);

    txt = i18n("<p>Some cameras automatically store generic comments in each image. "
               "These comments can be ignored automatically.</p>"
               "<p>Enter the comments that you want to ignore in the input field, one per line. "
               "Be sure to add the exact comment, including all whitespace.</p>");
    m_stripEXIFComments->setWhatsThis(txt);
    m_commentsToStrip->setWhatsThis(txt);
}

void Settings::GeneralPage::loadSettings( Settings::SettingsData* opt )
{
    m_trustTimeStamps->setCurrentIndex( opt->tTimeStamps() );
    m_useEXIFRotate->setChecked( opt->useEXIFRotate() );
    m_useEXIFComments->setChecked( opt->useEXIFComments() );

    m_stripEXIFComments->setChecked( opt->stripEXIFComments() );
    m_stripEXIFComments->setEnabled( opt->useEXIFComments() );

    QStringList commentsToStrip = opt->EXIFCommentsToStrip();
    QString commentsToStripStr;
    for (int i = 0; i < commentsToStrip.size(); ++i) {
        if (commentsToStripStr.size() > 0) {
            commentsToStripStr += QString::fromLatin1("\n");
        }
        commentsToStripStr += commentsToStrip.at(i);
    }
    m_commentsToStrip->setPlainText(commentsToStripStr);
    m_commentsToStrip->setEnabled( opt->stripEXIFComments() );

    m_useRawThumbnail->setChecked( opt->useRawThumbnail() );
    setUseRawThumbnailSize(QSize(opt->useRawThumbnailSize().width(), opt->useRawThumbnailSize().height()));
    m_barWidth->setValue( opt->histogramSize().width() );
    m_barHeight->setValue( opt->histogramSize().height() );
    m_showHistogram->setChecked( opt->showHistogram() );
    m_showSplashScreen->setChecked( opt->showSplashScreen() );
    m_listenForAndroidDevicesOnStartup->setChecked(opt->listenForAndroidDevicesOnStartup());
    DB::CategoryPtr cat = DB::ImageDB::instance()->categoryCollection()->categoryForName( opt->albumCategory() );
    if ( !cat )
        cat = DB::ImageDB::instance()->categoryCollection()->categories()[0];

    m_albumCategory->setEditText(cat->name());
}

void Settings::GeneralPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setTTimeStamps( (TimeStampTrust) m_trustTimeStamps->currentIndex() );
    opt->setUseEXIFRotate( m_useEXIFRotate->isChecked() );
    opt->setUseEXIFComments( m_useEXIFComments->isChecked() );

    opt->setStripEXIFComments(m_stripEXIFComments->isChecked());

    QStringList commentsToStrip = m_commentsToStrip->toPlainText().split(QString::fromLatin1("\n"));
    // Put the processable list to opt
    opt->setEXIFCommentsToStrip(commentsToStrip);

    QString commentsToStripString;
    for ( QString comment : commentsToStrip )
    {
        // separate comments with "-,-" and escape existing commas by doubling
        if ( !comment.isEmpty() )
            commentsToStripString += comment.replace( QString::fromLatin1(","), QString::fromLatin1(",,") ) + QString::fromLatin1("-,-");
    }
    // Put the storable list to opt
    opt->setCommentsToStrip(commentsToStripString);

    opt->setUseRawThumbnail( m_useRawThumbnail->isChecked() );
    opt->setUseRawThumbnailSize(QSize(useRawThumbnailSize()));
    opt->setShowHistogram( m_showHistogram->isChecked() );
    opt->setShowSplashScreen( m_showSplashScreen->isChecked() );
    opt->setListenForAndroidDevicesOnStartup(m_listenForAndroidDevicesOnStartup->isChecked());

    QString name = m_albumCategory->currentText();
    if (name.isNull()) {
        name = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
    }
    opt->setAlbumCategory(name);

    opt->setHistogramSize(QSize(m_barWidth->value(), m_barHeight->value()));
}

void Settings::GeneralPage::setUseRawThumbnailSize( const QSize& size  )
{
    m_useRawThumbnailWidth->setValue( size.width() );
    m_useRawThumbnailHeight->setValue( size.height() );
}

QSize Settings::GeneralPage::useRawThumbnailSize()
{
    return QSize( m_useRawThumbnailWidth->value(), m_useRawThumbnailHeight->value() );
}

void Settings::GeneralPage::showHistogramChanged( int state ) const
{
    MainWindow::Window::theMainWindow()->setHistogramVisibilty( state == Qt::Checked );
}

void Settings::GeneralPage::useEXIFCommentsChanged(int state)
{
    m_stripEXIFComments->setEnabled(state);
    m_commentsToStrip->setEnabled(state && m_stripEXIFComments->isChecked() );
}

void Settings::GeneralPage::stripEXIFCommentsChanged(int state)
{
    m_commentsToStrip->setEnabled(state);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
