#include "ExifPage.h"
#include "SettingsData.h"
#include <QTextCodec>
#include <klocale.h>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "config-kpa-exiv2.h"
#ifdef HAVE_EXIV2
#  include "Exif/Info.h"
#  include "Exif/TreeView.h"
#endif

Settings::ExifPage::ExifPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* vlay = new QVBoxLayout( this );
    QHBoxLayout* hlay1 = new QHBoxLayout();
    QHBoxLayout* hlay2 = new QHBoxLayout();
    vlay->addLayout( hlay1 );
    vlay->addLayout( hlay2 );

    _exifForViewer = new Exif::TreeView( i18n( "EXIF/IPTC info to show in the Viewer" ), this );
    hlay1->addWidget( _exifForViewer );

    _exifForDialog = new Exif::TreeView( i18n("EXIF/IPTC info to show in the EXIF dialog"), this );
    hlay1->addWidget( _exifForDialog );

    QLabel* _iptcCharsetLabel = new QLabel( i18n("Character set for image metadata:"), this );
    _iptcCharset = new QComboBox( this );
    QStringList _charsets;
    QList<QByteArray> _charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = _charsetsBA.constBegin(); it != _charsetsBA.constEnd(); ++it )
        _charsets << QString::fromLatin1(*it);
    _iptcCharset->insertItems( _iptcCharset->count(), _charsets );

    hlay2->addStretch( 1 );
    hlay2->addWidget( _iptcCharsetLabel );
    hlay2->addWidget( _iptcCharset );

}

void Settings::ExifPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setExifForViewer( _exifForViewer->selected() ) ;
    opt->setExifForDialog( _exifForDialog->selected() ) ;
    opt->setIptcCharset( _iptcCharset->currentText() );

}

void Settings::ExifPage::loadSettings( Settings::SettingsData* opt )
{
    _exifForViewer->reload();
    _exifForDialog->reload();
    _exifForViewer->setSelectedExif( Settings::SettingsData::instance()->exifForViewer() );
    _exifForDialog->setSelectedExif( Settings::SettingsData::instance()->exifForDialog() );
    _iptcCharset->setCurrentIndex( qMax( 0, QTextCodec::availableCodecs().indexOf( opt->iptcCharset().toAscii() ) ) );
}
