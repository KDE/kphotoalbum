#include "htmlexportdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qfile.h>
#include <qapplication.h>
#include <qeventloop.h>
#include "imagemanager.h"
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "options.h"
#include <qprogressdialog.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qhgroupbox.h>
#include <kstandarddirs.h>
#include <krun.h>

class MyCheckBox :public QCheckBox {

public:
    MyCheckBox( int width, const QString& text, QWidget* parent )
        :QCheckBox( text, parent ), _width( width )
        {
        }
    int width() const {
        return _width;
    }

private:
    int _width;
};


HTMLExportDialog::HTMLExportDialog( const ImageInfoList& list, QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("HTML Export"), Ok|Cancel, Ok, parent, name ), _list(list)
{
    QWidget* generalPage = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( generalPage, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2 );

    QLabel* label = new QLabel( i18n("Page Title"), generalPage );
    lay2->addWidget( label, 0, 0 );
    _title = new KLineEdit( generalPage );
    lay2->addWidget( _title, 0, 1 );

    // Thumbnail size
    label = new QLabel( i18n("Thumbnail size"), generalPage );
    lay2->addWidget( label, 1, 0 );

    QHBoxLayout* lay3 = new QHBoxLayout( 0 );
    lay2->addLayout( lay3, 1, 1 );

    _thumbSize = new QSpinBox( 16, 256, 1, generalPage );
    _thumbSize->setValue( 128 );
    lay3->addWidget( _thumbSize );
    lay3->addStretch(1);

    // Number of columns
    label = new QLabel( i18n("Number of Columns"), generalPage );
    lay2->addWidget( label, 2, 0 );

    QHBoxLayout* lay4 = new QHBoxLayout( (QWidget*)0, 0, 6 );
    lay2->addLayout( lay4, 2, 1 );

    QSpinBox* number = new QSpinBox( 1, 10, 1, generalPage );
    lay4->addWidget( number );

    _numOfCols = new QSlider( 1, 10, 1, 5, Horizontal, generalPage );
    _numOfCols->setTickmarks( QSlider::Below );
    lay4->addWidget( _numOfCols );

    connect( _numOfCols, SIGNAL( valueChanged( int ) ), number, SLOT( setValue(int) ) );
    connect( number, SIGNAL( valueChanged( int ) ), _numOfCols, SLOT( setValue(int) ) );
    number->setValue( _numOfCols->value() );

    // Generate Tooltips
    _generateToolTips = new QCheckBox( i18n( "Generate tool tips" ), generalPage );
    _generateToolTips->setChecked( true );
    lay2->addMultiCellWidget( _generateToolTips, 3, 3, 0, 1 );

    // Seperator
    QFrame* sep = new QFrame( generalPage );
    sep->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    lay2->addMultiCellWidget( sep, 4, 4, 0, 1 );

    // Base Directory
    label = new QLabel( i18n("Base Directory"), generalPage );
    lay2->addWidget( label, 5, 0 );

    QHBoxLayout* lay5 = new QHBoxLayout( (QWidget*)0, 0, 6 );
    lay2->addLayout( lay5, 5, 1 );

    _baseDir = new KLineEdit( generalPage );
    lay5->addWidget( _baseDir );

    QPushButton* but = new QPushButton( QString::fromLatin1( ".." ), generalPage );
    lay5->addWidget( but );
    but->setFixedWidth( 25 );

    connect( but, SIGNAL( clicked() ), this, SLOT( selectDir() ) );
    _baseDir->setText( Options::instance()->HTMLBaseDir() );

    // Base URL
    label = new QLabel( i18n("Base URL"), generalPage );
    lay2->addWidget( label, 6, 0 );

    _baseURL = new KLineEdit( generalPage );
    _baseURL->setText( Options::instance()->HTMLBaseURL() );
    lay2->addWidget( _baseURL, 6, 1 );

    // Output Directory
    label = new QLabel( i18n("Output Directory"), generalPage );
    lay2->addWidget( label, 7, 0 );
    _outputDir = new KLineEdit( generalPage );
    lay2->addWidget( _outputDir, 7, 1 );

    // Image sizes
    QHGroupBox* sizes = new QHGroupBox( i18n("Image sizes"), generalPage );
    lay1->addWidget( sizes );
    QWidget* content = new QWidget( sizes );
    QGridLayout* lay6 = new QGridLayout( content, 2, 4 );
    lay6->setAutoAdd( true );
    MyCheckBox* size320  = new MyCheckBox( 320, "320x200", content );
    MyCheckBox* size640  = new MyCheckBox( 640, "640x480", content );
    MyCheckBox* size800  = new MyCheckBox( 800, "800x600", content );
    MyCheckBox* size1024 = new MyCheckBox( 1024, "1024x768", content );
    MyCheckBox* size1280 = new MyCheckBox( 1280, "1280x1024", content );
    MyCheckBox* size1600 = new MyCheckBox( 1600, "1600x1200", content );
    MyCheckBox* sizeOrig = new MyCheckBox( -1, "Full Size", content );

    _cbs << size320 << size640 << size800 << size1024 << size1280 << size1600 << sizeOrig;

    resize( 500, sizeHint().height() );
}

bool HTMLExportDialog::generate()
{
    bool generateTooltips = _generateToolTips->isChecked();

    int count = 0;
    for( QValueList<MyCheckBox*>::Iterator it2 = _cbs.begin(); it2 != _cbs.end(); ++it2 ) {
        if ( (*it2)->isChecked() )
            count++;
    }

    _total = _waitCounter = _list.count() * ( 1 + count ); // 1 thumbnail + 1 real image
    QString dir = outputDir( true );
    if ( dir.isNull() ) {
        return false;
    }

    _progress = new QProgressDialog( i18n("Generating images for HTML page "), i18n("Cancel"), _total, this );
    _progress->setProgress( 0 );
    connect( _progress, SIGNAL( cancelled() ), this, SLOT( slotCancelGenerate() ) );


    // Gotta be after the creation of _progress.
    if ( count == 0 ) {
        QMessageBox::critical( this, i18n("No image sizes selected"), i18n("No image sizes were selected. Please select at least one"),
                               QMessageBox::Ok, 0 );
        return false;
    }

    bool success = true;
    QDir directory(dir);
    if ( !directory.exists() ) {
        success = directory.mkdir(dir);
    }
    if ( !success ) {
        QMessageBox::critical( this, i18n("Couldn't create directory"), i18n("Couldn't create directory '%1'").arg(dir),
                               QMessageBox::Ok, 0 );
        return false;
    }


    QString index = dir + QString::fromLatin1("/") + "index.html";
    QFile file(index);
    if ( !file.open(IO_WriteOnly) ) {
        QMessageBox::critical( this, i18n("Couldn't create file"), i18n("Couldn't create file '%1'").arg(index),
                               QMessageBox::Ok, 0 );
        return false;
    }

    hide();

    // Generate HTML
    QDomDocument doc;
    QDomElement top = doc.createElement( QString::fromLatin1( "html" ));
    doc.appendChild( top );

    QDomElement head = doc.createElement( QString::fromLatin1( "head" ) );
    top.appendChild( head );

    QDomElement title = doc.createElement( QString::fromLatin1( "title" ) );
    head.appendChild( title );
    QDomText text = doc.createTextNode( _title->text() );
    title.appendChild( text );

    if ( generateTooltips ) {
        QDomElement script = doc.createElement( QString::fromLatin1( "script" ) );
        script.setAttribute( QString::fromLatin1( "language" ), QString::fromLatin1( "JavaScript" ) );
        script.setAttribute( QString::fromLatin1( "type" ), QString::fromLatin1( "text/javascript" ) );
        script.setAttribute( QString::fromLatin1( "src" ), QString::fromLatin1( "infobox.js" ) );
        head.appendChild( script );

        // Without this the script do not work in konqueror
        script.appendChild( doc.createTextNode( QString::fromLatin1( "" ) ) );

        QDomElement div = doc.createElement( QString::fromLatin1( "div" ) );
        div.setAttribute( QString::fromLatin1( "id" ), QString::fromLatin1( "infodiv" ) );
        div.setAttribute( QString::fromLatin1( "style" ), QString::fromLatin1( "position:absolute; visibility:hidden; z-index:20; top:0px; left:0px;" ) );
        head.appendChild( div );

        // Without this konqueror do not show anything
        div.appendChild( doc.createTextNode( QString::fromLatin1( "" ) ) );
    }

    QDomElement body = doc.createElement( QString::fromLatin1( "body" ) );
    top.appendChild( body );

    title = doc.createElement( QString::fromLatin1( "h1" ) );
    body.appendChild( title );
    text = doc.createTextNode( _title->text() );
    title.appendChild( text );

    QDomElement table = doc.createElement( QString::fromLatin1( "table" ) );
    body.appendChild( table );

    int i = 0;
    int cols = _numOfCols->value();
    QDomElement row;
    for( ImageInfoListIterator it( _list ); *it; ++it ) {
        if ( _progress->wasCancelled() ) {
            break;
        }

        if ( i % cols == 0 ) {
            row = doc.createElement( QString::fromLatin1( "tr" ) );
            table.appendChild( row );
            i = 0;
        }

        bool anyTips = !(*it)->description().isEmpty();

        QDomElement col = doc.createElement( QString::fromLatin1( "td" ) );
        col.setAttribute( QString::fromLatin1( "align" ), QString::fromLatin1( "center" ) );
        row.appendChild( col );

        QString name = createImage( *it, _thumbSize->value() );
        QDomElement img = doc.createElement( QString::fromLatin1( "img" ) );
        img.setAttribute( QString::fromLatin1( "src" ), name );
        col.appendChild( img );
        if ( generateTooltips && anyTips ) {
            img.setAttribute( QString::fromLatin1( "onMouseOver" ),
                            QString::fromLatin1( "tip('%1'); return true;").arg( (*it)->fileName(true) ) );
            img.setAttribute( QString::fromLatin1( "onMouseOut" ), QString::fromLatin1( "untip(); return true;" ) );
        }

        col.appendChild( doc.createElement( QString::fromLatin1( "br" ) ) );

        for( QValueList<MyCheckBox*>::Iterator it2 = _cbs.begin(); it2 != _cbs.end(); ++it2 ) {
            if ( (*it2)->isChecked() ) {
                name = createImage( *it, (*it2)->width() );
                QDomElement a = doc.createElement( QString::fromLatin1( "a" ) );
                a.setAttribute( QString::fromLatin1( "href" ), name );

                col.appendChild( a );
                a.appendChild( doc.createTextNode( (*it2)->text() ) );
            }
        }


        if ( generateTooltips && anyTips ) {
            QDomElement script = doc.createElement( QString::fromLatin1( "script" ) );
            script.setAttribute( QString::fromLatin1( "language" ), QString::fromLatin1( "JavaScript" ) );
            script.setAttribute( QString::fromLatin1( "TYPE" ), QString::fromLatin1( "text/javascript" ) );
            col.appendChild( script );

            QString desc = (*it)->description();
            desc.replace( '\n', ' ' );
            QString text = QString::fromLatin1( "maketip('%1','Description','%2');" ).arg( (*it)->fileName(true) )
                           .arg( desc );
            QDomText txt = doc.createTextNode( text );
            script.appendChild( txt );
        }
        else {
            col.appendChild( doc.createElement( QString::fromLatin1( "br" ) ) );
            col.appendChild( doc.createTextNode( (*it)->description() ) );
        }

        ++i;
    }

    if ( !_progress->wasCancelled() ) {
        QTextStream stream( &file );
        stream << doc.toString();
        file.close();

        if ( _waitCounter > 0 )
            qApp->eventLoop()->enterLoop();
    }

    // Copy infobox.js to the dest dir.
    if ( generateTooltips ) {
        QString infofile = locate("data",QString::fromLatin1( "kpalbum/infobox.js" ) );
        QFile f1( infofile );
        if ( !f1.open( IO_ReadOnly ) ) {
            QMessageBox::warning( this, i18n("Unable to read infobox.js"),
                                  i18n("Unable to read file %1").arg( infofile ), QMessageBox::Ok, 0 );
        }
        else {
            QTextStream s(&f1);
            QString data = s.read();
            f1.close();

            QString outfile = dir + QString::fromLatin1("/") + "infobox.js";
            QFile f2( outfile );
            if ( !f2.open( IO_WriteOnly ) ) {
                QMessageBox::warning( this, i18n("Unable to write infobox.js"),
                                      i18n("Unable to write file %1").arg( outfile ), QMessageBox::Ok, 0 );
            }
            else {
                QTextStream s2( &f2 );
                s2 << data;
                f2.close();
            }
        }
    }

    return true;
}

QString HTMLExportDialog::createImage( ImageInfo* info, int size )
{
    QFileInfo finfo(info->fileName(true));
    QString baseName = finfo.baseName();

    ImageManager::instance()->load( info->fileName( false ),  this, info->angle(), size, size, false, true, false );
    QString name = baseName;
    if ( size != -1 )
        name += QString::fromLatin1( "-" ) + QString::number( size );
    name += QString::fromLatin1( ".jpg" ) ;
    return name;
}

void HTMLExportDialog::pixmapLoaded( const QString& fileName, int width, int height, int /*angle*/, const QImage& image )
{
    _waitCounter--;

    QString dir = outputDir( false );
    if ( !dir.isNull() ) {
        QFileInfo finfo(fileName);
        QString baseName = finfo.baseName();
        QString ext = finfo.extension();
        int size = QMAX( width, height );

        QString file = dir + QString::fromLatin1( "/" ) + baseName;
        if ( size != -1 )
            file += QString::fromLatin1( "-" ) + QString::number( size );
        file += QString::fromLatin1( ".jpg" );

        bool success = image.save( file, "JPEG" );
        if ( !success ) {
            QMessageBox::warning( this, i18n("Unable to write image"), i18n("Unable to write image '%1'").arg(file), QMessageBox::Ok, 0 );
        }
    }
    else
        Q_ASSERT( false );


    _progress->setProgress( _total - _waitCounter );
    qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

    if ( _waitCounter == 0 ) {
        qApp->eventLoop()->exitLoop();
    }
}

void HTMLExportDialog::slotOk()
{
    bool ok = generate();
    if ( ok ) {
        Options::instance()->setHTMLBaseDir( _baseDir->text() );
        Options::instance()->setHTMLBaseURL( _baseURL->text() );
        Options::instance()->save();
        accept();
        if ( ! _baseURL->text().isEmpty() )
            new KRun( _baseURL->text() + QString::fromLatin1( "/" ) + _outputDir->text() );
    }
    else {
        _progress->cancel();
        _progress->reset();
    }
}

void HTMLExportDialog::selectDir()
{
    QString dir = QFileDialog::getExistingDirectory( _baseDir->text(), this );
    if ( !dir.isNull() )
        _baseDir->setText( dir );
}

QString HTMLExportDialog::outputDir( bool showErr )
{
    QDir dir( _baseDir->text() );
    if ( _baseDir->text().isEmpty() || !dir.exists() ) {
        if ( showErr )
            QMessageBox::critical( this, i18n("Specified Base directory is invalid"),
                                   i18n("<qt>You did not specify a directory for base, or the specified "
                                        "directory do not exists</qt>"), QMessageBox::Ok, 0, 0 );
        return QString::null;
    }

    if ( _outputDir->text().isEmpty() ) {
        if ( showErr )
            QMessageBox::critical( this, i18n("Empty output directory"),
                                   i18n("<qt>You did not specify a directory for the output</qt>"), QMessageBox::Ok, 0, 0 );
        return QString::null;
    }

    QString str = _baseDir->text() + QString::fromLatin1( "/" ) + _outputDir->text();
    QDir dir2( str );
    if ( dir2.exists() ) {
        int answer = QMessageBox::Yes;
        if ( showErr )
            answer = QMessageBox::warning( this, i18n("Directory exists"), i18n("<qt>Output directory specified already exists "
                                                                                "Normally you should specify a new directory, "
                                                                                "Contiune?</qt>"),
                                           QMessageBox::No, QMessageBox::Yes );
        if ( answer == QMessageBox::No )
            return QString::null;
    }

    return str;
}

void HTMLExportDialog::slotCancelGenerate()
{
    ImageManager::instance()->stop( this );
    _waitCounter = 0;
}

#include "htmlexportdialog.moc"
