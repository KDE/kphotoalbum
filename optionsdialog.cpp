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

#include "optionsdialog.h"
#include <kfiledialog.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <qspinbox.h>
#include "options.h"
#include <kicondialog.h>
#include <qlistbox.h>
#include <kmessagebox.h>
#include "imagedb.h"
#include <qcheckbox.h>
#include <kinputdialog.h>
#include <qwhatsthis.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qvgroupbox.h>
#include <qhbox.h>
#include "viewersizeconfig.h"
#include <limits.h>
#ifdef HASKIPI
#  include <libkipi/pluginloader.h>
#endif
#include <kdebug.h>
#include <kcolorbutton.h>
#include "categorycollection.h"
#include "showbusycursor.h"
#include "optionsdialog.moc"
#include <kapplication.h>
#include <kconfig.h>
#include "mainview.h"

#ifdef HASEXIV2
#  include "Exif/Info.h"
#  include "Exif/TreeView.h"
#endif

#include <qlistview.h>
#include <config.h>

OptionsDialog::OptionsDialog( QWidget* parent, const char* name )
    :KDialogBase( IconList, i18n( "Options" ), Apply | Ok | Cancel, Ok, parent, name, false ), _memberMap( MemberMap( ImageDB::instance() ) ), _currentCategory( QString::null ), _currentGroup( QString::null )
{
    createGeneralPage();
    createThumbNailPage();
    createOptionGroupsPage();
    createGroupConfig();
    createViewerPage();
    createPluginPage();
    createEXIFPage();

    connect( this, SIGNAL( aboutToShowPage( QWidget* ) ), this, SLOT( slotPageChange() ) );
    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotMyOK() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotMyOK() ) );
}

void OptionsDialog::createGeneralPage()
{
    QWidget* top = addPage( i18n("General" ), i18n("General" ),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kimdaba" ),
                                                             KIcon::Desktop, 32 ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    QVGroupBox* box = new QVGroupBox( i18n( "New Images" ), top );
    lay1->addWidget( box );

    // Thrust time stamps
    QWidget* container = new QWidget( box );
    QLabel* timeStampLabel = new QLabel( i18n("Trust image dates:"), container );
    _trustTimeStamps = new KComboBox( container );
    _trustTimeStamps->insertStringList( QStringList() << i18n("Always") << i18n("Ask") << i18n("Never") );
    QHBoxLayout* hlay = new QHBoxLayout( container, 0, 6 );
    hlay->addWidget( timeStampLabel );
    hlay->addWidget( _trustTimeStamps );
    hlay->addStretch( 1 );

    // Do EXIF rotate
    _useEXIFRotate = new QCheckBox( i18n( "Use EXIF orientation information" ), box );

    _useEXIFComments = new QCheckBox( i18n( "Use EXIF description" ), box );

    // Search for images on startup
    _searchForImagesOnStartup = new QCheckBox( i18n("Search for new images on startup"), box );

    // Datebar size
    container = new QWidget( top );
    lay1->addWidget( container );
    hlay = new QHBoxLayout( container, 0, 6 );
    QLabel* datebarSize = new QLabel( i18n("Size of histogram boxes in datebar:"), container );
    hlay->addWidget( datebarSize );
    _barWidth = new QSpinBox( 15, 100, 1, container );
    hlay->addWidget( _barWidth );
    QLabel* cross = new QLabel( QString::fromLatin1( " x " ), container );
    hlay->addWidget( cross );
    _barHeight = new QSpinBox( 15, 100, 1, container );
    hlay->addWidget( _barHeight );
    hlay->addStretch( 1 );

    // Compressed index.xml
    _compressedIndexXML = new QCheckBox( i18n("Use compressed index.xml file"), top );
    lay1->addWidget( _compressedIndexXML );

    // Auto save
    QLabel* label = new QLabel( i18n("Auto save every:"), top );
    _autosave = new QSpinBox( 1, 120, 1, top );
    _autosave->setSuffix( i18n( "min." ) );
    QHBoxLayout* lay6 = new QHBoxLayout( lay1, 6 );
    lay6->addWidget( label );
    lay6->addWidget( _autosave );
    lay6->addStretch( 1 );

    // Backup
    hlay = new QHBoxLayout( lay1 );
    QLabel* backupLabel = new QLabel( i18n( "Number of backups to keep" ), top );
    hlay->addWidget( backupLabel );

    _backupCount = new QSpinBox( -1, 100, 1, top );
    _backupCount->setSpecialValueText( i18n( "Infinite" ) );
    hlay->addWidget( _backupCount );

    _compressBackup = new QCheckBox( i18n( "Compress backup file" ), top );
    lay1->addWidget( _compressBackup );

    // Album Category
    QLabel* albumCategoryLabel = new QLabel( i18n("Category for virtual albums:" ), top, "albumCategoryLabel" );
    _albumCategory = new QComboBox( top, "_albumCategory" );
    QHBoxLayout* lay7 = new QHBoxLayout( lay1, 6 );
    lay7->addWidget( albumCategoryLabel );
    lay7->addWidget( _albumCategory );
    QValueList<CategoryPtr> categories = ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        _albumCategory->insertItem( (*it)->text() );
    }

    lay1->addStretch( 1 );


    // Whats This
    QString txt;

    txt = i18n( "<qt><p>KimDaBa will try to read the image date from EXIF information in the image. "
                "If that fails it will try to get the date from the file's time stamp.</p>"
                "<p>However, this information will be wrong if the image was scanned in (you want the date the image "
                "was taken, not the date of the scan).</p>"
                "<p>If you only scan images, in contrast to sometimes using "
                "a digital camera, you should reply <b>no</b>. If you never scan images, you should reply <b>yes</b>, "
                "otherwise reply <b>ask</b>. This will allow you to decide whether the images are from "
                "the scanner or the camera, from session to session.</p></qt>" );
    QWhatsThis::add( timeStampLabel, txt );
    QWhatsThis::add( _trustTimeStamps, txt );

    txt = i18n( "<qt><p>JPEG images may contain information about rotation. "
                "If you have a reason for not using this information to get a default rotation of "
                "your images, uncheck this check box.</p>"
                "<p>Note: Your digital camera may not write this information into the images at all.</p></qt>" );
    QWhatsThis::add( _useEXIFRotate, txt );

    txt = i18n( "<qt><p>JPEG images may contain a description."
               "Using this checkbox you specify if you want to use this as a "
               "default description for your images.</p></qt>" );
    QWhatsThis::add( _useEXIFComments, txt );

    txt = i18n( "<qt><p>KimDaBa is capable of searching for new images itself when started, this does, "
                "however, take some time, so instead you may wish to manually tell KimDaBa to search for new images "
                "using <tt>Maintenance->Rescan for new images</tt></qt>");
    QWhatsThis::add( _searchForImagesOnStartup, txt );

    txt = i18n("<qt><p>KimDaBa shares plugins with other imaging applications, some of which have the concept of albums. "
               "KimDaBa do not have this concept; nevertheless, for certain plugins to function, KimDaBa behaves "
               "to the plugin system as if it did.</p>"
               "<p>KimDaBa does this by defining the current album to be the current view - that is, all the images the "
               "browser offers to display.</p>"
               "<p>In addition to the current album, KimDaBa must also be able to give a list of all albums; "
               "the list of all albums is defined in the following way:"
               "<ul><li>When KimDaBa's browser displays the content of a category, say all Persons, then each item in this category "
               "will look like an album to the plugin."
               "<li>Otherwise, the category you specify using this option will be used; e.g. if you specify Persons "
               "with this option, then KimDaBa will act as if you had just chosen to display persons and then invoke "
               "the plugin which needs to know about all albums.</p>"
               "<p>Most users would probably want to specify Keywords here.</p></qt>");
    QWhatsThis::add( albumCategoryLabel, txt );
    QWhatsThis::add( _albumCategory, txt );

    txt = i18n("<qt><p>KimDaBa has the possibility to back up the index.xml file by keeping copies named index.xml~1~ index.xml~2~ etc."
               "using the spinbox specify the amount of backup files to keep - KimDaBa will delete the oldest backup file when it reaches "
               "the maximum amount of backup files.</p>"
               "<p>The index.xml file may grow large if you have many images, and in that case it is useful to ask KimDaBa to zip "
               "the backup files to preserve disk space.</p></qt>" );
    QWhatsThis::add( backupLabel, txt );
    QWhatsThis::add( _backupCount, txt );
    QWhatsThis::add( _compressBackup, txt );

    txt = i18n( "<qt>KimDaBa is using a single index.xml file as its <i>data base</i>. With lots of images it may take "
                "a long time to read this file. You may cut down this time by approaximately a factor of 2 by checking this check box. "
                "The disadvantage is that the index.xml file is less readable by human eyes.</qt>");
    QWhatsThis::add( _compressedIndexXML, txt );
}

void OptionsDialog::createThumbNailPage()
{
    QWidget* top = addPage( i18n("Thumbnail View" ), i18n("Thumbnail View" ),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "view_icon" ),
                                                             KIcon::Desktop, 32 ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );
    QHBoxLayout* hlay = new QHBoxLayout( lay1, 6 );

    // Preview size
    QLabel* previewSizeLabel = new QLabel( i18n("Preview image size:" ), top, "previewSizeLabel" );
    _previewSize = new QSpinBox( 0, 2000, 10, top, "_previewSize" );
    _previewSize->setSpecialValueText( i18n("No Image Preview") );
    hlay->addWidget( previewSizeLabel );
    hlay->addWidget( _previewSize );
    hlay->addStretch( 1 );

    QHBoxLayout* lay4 = new QHBoxLayout( lay1, 6 );
    lay4->addStretch(1);

    // Display Labels
    _displayLabels = new QCheckBox( i18n("Display labels in thumbnail view" ), top, "displayLabels" );
    lay1->addWidget( _displayLabels );

    // Auto Show Thumbnail view
    QLabel* autoShowLabel = new QLabel( i18n("Automatically show thumbnail view when images matches gets below: "), top );
    _autoShowThumbnailView = new QSpinBox( 0, 10000, 10, top );
    _autoShowThumbnailView->setSpecialValueText( i18n("Never") );
    lay4 = new QHBoxLayout( lay1, 6 );
    lay4->addWidget( autoShowLabel );
    lay4->addWidget( _autoShowThumbnailView );


    lay1->addStretch(1);

    // Whats This
    QString txt;

    txt = i18n( "<qt><p>If you select <tt>Help|Show Tooltips</tt> in the thumbnail view, then you will see a small tool tip window "
                "displaying information about the thumbnails. This window includes a small preview image. "
                "This option configures the image size</p></qt>" );
    QWhatsThis::add( previewSizeLabel, txt );
    QWhatsThis::add( _previewSize, txt );


    txt = i18n("<qt>Checking this option will show the base name for the file under "
               "thumbnails in the thumbnail view</qt>");
    QWhatsThis::add( _displayLabels, txt );

    txt = i18n("<qt><p>When you are browsing the images, and the count gets below the value specified here, "
               "the thumbnails will be shown automatically. The alternative is to continue showing the "
               "browser until you press <i>Show Images</i>.<p>"
               "<p>With this option on, you can choose to see the browser by pressing ctrl+mouse button.<br>"
               "With this option off, you can choose to see the images by pressing ctrl+mouse button.</p></qt>");
    QWhatsThis::add( _autoShowThumbnailView, txt );
    QWhatsThis::add( autoShowLabel, txt );
}


class OptionGroupItem :public QListBoxText
{
public:
    OptionGroupItem( const QString& category, const QString& text, const QString& icon,
                     Category::ViewSize size, Category::ViewType type, QListBox* parent );
    void setLabel( const QString& label );

    QString _categoryOrig, _textOrig, _iconOrig;
    QString _text, _icon;
    Category::ViewSize _size, _sizeOrig;
    Category::ViewType _type, _typeOrig;
};

OptionGroupItem::OptionGroupItem( const QString& category, const QString& text, const QString& icon,
                                  Category::ViewSize size, Category::ViewType type, QListBox* parent )
    :QListBoxText( parent, text ),
     _categoryOrig( category ), _textOrig( text ), _iconOrig( icon ),
     _text( text ), _icon( icon ), _size( size ), _sizeOrig( size ), _type( type ), _typeOrig( type )
{
}

void OptionGroupItem::setLabel( const QString& label )
{
    setText( label );
    _text = label;

    // unfortunately setText do not call updateItem, so we need to force it with this hack.
    bool b = listBox()->isSelected( this );
    listBox()->setSelected( this, !b );
    listBox()->setSelected( this, b );
}


void OptionsDialog::createOptionGroupsPage()
{
    QWidget* top = addPage( i18n("Categories"), i18n("Categories"),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "identity" ),
                                                             KIcon::Desktop, 32 ) );

    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );

    _categories = new QListBox( top );
    connect( _categories, SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( edit( QListBoxItem* ) ) );
    lay2->addWidget( _categories );

    QVBoxLayout* lay3 = new QVBoxLayout( lay2, 6 );
    QLabel* label = new QLabel( i18n( "Label:" ), top );
    lay3->addWidget( label );

    _text = new QLineEdit( top );
    connect( _text, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( slotLabelChanged( const QString& ) ) );

    lay3->addWidget( _text );

    _icon = new KIconButton(  top );
    QHBoxLayout* lay5 = new QHBoxLayout( lay3 );
    lay5->addWidget( _icon );
    lay5->addStretch(1);
    _icon->setIconSize(32);
    _icon->setIcon( QString::fromLatin1( "personsIcon" ) );
    connect( _icon, SIGNAL( iconChanged( QString ) ), this, SLOT( slotIconChanged( QString ) ) );
    lay3->addStretch(1);

    // Prefered View
    _preferredViewLabel = new QLabel( i18n("Preferred view:"), top );
    lay3->addWidget( _preferredViewLabel );

    _preferredView = new QComboBox( top );
    lay3->addWidget( _preferredView );
    QStringList list;
    list << i18n("Small List View") << i18n("Large List View") << i18n("Small Icon View") << i18n("Large Icon View");
    _preferredView->insertStringList( list );
    connect( _preferredView, SIGNAL( activated( int ) ), this, SLOT( slotPreferredViewChanged( int ) ) );

    QHBoxLayout* lay4 = new QHBoxLayout( lay1, 6 );
    KPushButton* newItem = new KPushButton( i18n("New"), top );
    connect( newItem, SIGNAL( clicked() ), this, SLOT( slotNewItem() ) );

    _delItem = new KPushButton( i18n("Delete"), top );
    connect( _delItem, SIGNAL( clicked() ), this, SLOT( slotDeleteCurrent() ) );

    lay4->addStretch(1);
    lay4->addWidget( newItem );
    lay4->addWidget( _delItem );

    _current = 0;
}



void OptionsDialog::show()
{
    Options* opt = Options::instance();

    // General page
    _previewSize->setValue( opt->previewSize() );
    _trustTimeStamps->setCurrentItem( opt->tTimeStamps() );
    _useEXIFRotate->setChecked( opt->useEXIFRotate() );
    _useEXIFComments->setChecked( opt->useEXIFComments() );
    _searchForImagesOnStartup->setChecked( opt->searchForImagesOnStartup() );
    _compressedIndexXML->setChecked( opt->useCompressedIndexXML() );
    _autosave->setValue( opt->autoSave() );
    _barWidth->setValue( opt->histogramSize().width() );
    _barHeight->setValue( opt->histogramSize().height() );
    _backupCount->setValue( opt->backupCount() );
    _compressBackup->setChecked( opt->compressBackup() );

    CategoryPtr cat = ImageDB::instance()->categoryCollection()->categoryForName( opt->albumCategory() );
    if ( !cat )
        cat = ImageDB::instance()->categoryCollection()->categories()[0];
    _albumCategory->setCurrentText( cat->text() );

    _displayLabels->setChecked( opt->displayLabels() );
    _viewImageSetup->setSize( opt->viewerSize() );
    _viewImageSetup->setLaunchFullScreen( opt->launchViewerFullScreen() );
    _slideShowSetup->setSize( opt->slideShowSize() );
    _slideShowSetup->setLaunchFullScreen( opt->launchSlideShowFullScreen() );
    _slideShowInterval->setValue( opt->slideShowInterval() );
    _cacheSize->setValue( opt->viewerCacheSize() );
    _autoShowThumbnailView->setValue( opt->autoShowThumbnailView() );

    _delayLoadingPlugins->setChecked( opt->delayLoadingPlugins() );
    // Config Groups page
    _categories->clear();
    QValueList<CategoryPtr> categories = ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if( !(*it)->isSpecialCategory() ) {
            new OptionGroupItem( (*it)->name(), (*it)->text(),(*it)->iconName(),(*it)->viewSize(),(*it)->viewType(),_categories );
        }
    }

#ifdef HASEXIV2
    _exifForViewer->reload();
    _exifForDialog->reload();
    _exifForViewer->setSelected( Options::instance()->exifForViewer() );
    _exifForDialog->setSelected( Options::instance()->exifForDialog() );
#endif
    enableDisable( false );
    KDialogBase::show();
}



// KDialogBase has a slotOK which we do not want to override.
void OptionsDialog::slotMyOK()
{
    ShowBusyCursor dummy;
    Options* opt = Options::instance();

    // General
    opt->setPreviewSize( _previewSize->value() );
    opt->setTTimeStamps( (Options::TimeStampTrust) _trustTimeStamps->currentItem() );
    opt->setUseEXIFRotate( _useEXIFRotate->isChecked() );
    opt->setUseEXIFComments( _useEXIFComments->isChecked() );
    opt->setSearchForImagesOnStartup( _searchForImagesOnStartup->isChecked() );
    opt->setBackupCount( _backupCount->value() );
    opt->setCompressBackup( _compressBackup->isChecked() );
    opt->setUseCompressedIndexXML( _compressedIndexXML->isChecked() );
    opt->setAutoSave( _autosave->value() );
    QString name = ImageDB::instance()->categoryCollection()->nameForText( _albumCategory->currentText() );
    if ( name.isNull() )
        name = ImageDB::instance()->categoryCollection()->categoryNames()[0];
    opt->setHistogramSize( QSize( _barWidth->value(), _barHeight->value() ) );

    opt->setAlbumCategory( name );
    opt->setDisplayLabels( _displayLabels->isChecked() );
    opt->setViewerSize( _viewImageSetup->size() );
    opt->setLaunchViewerFullScreen( _viewImageSetup->launchFullScreen() );
    opt->setSlideShowInterval( _slideShowInterval->value() );
    opt->setViewerCacheSize( _cacheSize->value() );
    opt->setSlideShowSize( _slideShowSetup->size() );
    opt->setLaunchSlideShowFullScreen( _slideShowSetup->launchFullScreen() );
    opt->setAutoShowThumbnailView( _autoShowThumbnailView->value() );

    // ----------------------------------------------------------------------
    // Categories

    // Delete items
    for( QValueList<OptionGroupItem*>::Iterator it = _deleted.begin(); it != _deleted.end(); ++it ) {
        if ( !(*it)->_categoryOrig.isNull() ) {
            // the Options instance knows about the item.
            ImageDB::instance()->categoryCollection()->removeCategory( (*it)->_categoryOrig );
        }
    }

    // Created or Modified items
    for ( QListBoxItem* i = _categories->firstItem(); i; i = i->next() ) {
        OptionGroupItem* item = static_cast<OptionGroupItem*>( i );
        if ( item->_categoryOrig.isNull() ) {
            // New Item
            ImageDB::instance()->categoryCollection()->addCategory( item->_text, item->_icon, item->_size, item->_type  );
        }
        else {
            if ( item->_text != item->_textOrig ) {
                ImageDB::instance()->categoryCollection()->rename(  item->_categoryOrig, item->_text );
                item->_categoryOrig =item->_text;
            }
            if ( item->_icon != item->_iconOrig ) {
                ImageDB::instance()->categoryCollection()->categoryForName( item->_categoryOrig )->setIconName( item->_icon );
            }
            if ( item->_size != item->_sizeOrig ) {
                ImageDB::instance()->categoryCollection()->categoryForName( item->_categoryOrig )->setViewSize( item->_size );
            }
            if ( item->_type != item->_typeOrig ) {
                ImageDB::instance()->categoryCollection()->categoryForName( item->_categoryOrig )->setViewType( item->_type );
            }
        }
    }

    saveOldGroup();
    ImageDB::instance()->setMemberMap( _memberMap );

    // misc stuff
#ifdef HASKIPI
    _pluginConfig->apply();
    opt->setDelayLoadingPlugins( _delayLoadingPlugins->isChecked() );
#endif

    // EXIF
#ifdef HASEXIV2
    opt->setExifForViewer( _exifForViewer->selected() ) ;
    opt->setExifForDialog( _exifForDialog->selected() ) ;
#endif

    emit changed();
    kapp->config()->sync();
}


void OptionsDialog::edit( QListBoxItem* i )
{
    if ( i == 0 )
        return;

    OptionGroupItem* item = static_cast<OptionGroupItem*>(i);
    _current = item;
    _text->setText( item->_text );
    _icon->setIcon( item->_icon );
    _preferredView->setCurrentItem( (int) item->_size + 2 * (int) item->_type );
    enableDisable( true );
}

void OptionsDialog::slotLabelChanged( const QString& label)
{
    if( _current )
        _current->setLabel( label );
}

void OptionsDialog::slotPreferredViewChanged( int i )
{
    if ( _current ) {
        if ( i < 2 )
            _current->_type = Category::ListView;
        else
            _current->_type = Category::IconView;

        if ( i % 2 == 1 )
            _current->_size = Category::Large;
        else
            _current->_size = Category::Small;
    }
}



void OptionsDialog::slotIconChanged( QString icon )
{
    if( _current )
        _current->_icon = icon;
}

void OptionsDialog::slotNewItem()
{
    _current = new OptionGroupItem( QString::null, QString::null, QString::null, Category::Small, Category::ListView, _categories );
    _text->setText( QString::fromLatin1( "" ) );
    _icon->setIcon( QString::null );
    enableDisable( true );
    _categories->setSelected( _current, true );
    _text->setFocus();
}

void OptionsDialog::slotDeleteCurrent()
{
    int answer = KMessageBox::Yes;
    KMessageBox::questionYesNo( this, i18n("<qt>Really delete cateory '%1'?").arg( _current->_text) );
    if ( answer == KMessageBox::No )
        return;

    _deleted.append( _current );
    _categories->takeItem( _current );
    _current = 0;
    _text->setText( QString::fromLatin1( "" ) );
    _icon->setIcon( QString::null );
    enableDisable(false);
}

void OptionsDialog::enableDisable( bool b )
{
    _delItem->setEnabled( b );
    _text->setEnabled( b );
    _icon->setEnabled( b );
    _preferredViewLabel->setEnabled( b );
    _preferredView->setEnabled( b );
}

void OptionsDialog::createGroupConfig()
{
    QWidget* top = addPage( i18n("Member Groups" ), i18n("Member Groups" ),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kuser" ),
                                                             KIcon::Desktop, 32 ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    // Category
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
    QLabel* label = new QLabel( i18n( "Category:" ), top );
    lay2->addWidget( label );
    _category = new QComboBox( top );
    lay2->addWidget( _category );
    lay2->addStretch(1);

    QHBoxLayout* lay3 = new QHBoxLayout( lay1, 6 );

    // Groups
    QVBoxLayout* lay4 = new QVBoxLayout( lay3, 6 );
    label = new QLabel( i18n( "Groups:" ), top );
    lay4->addWidget( label );
    _groups = new QListBox( top );
    lay4->addWidget( _groups );

    // Members
    QVBoxLayout* lay5 = new QVBoxLayout( lay3, 6 );
    label = new QLabel( i18n( "Members:" ), top );
    lay5->addWidget( label );
    _members = new QListBox( top );
    lay5->addWidget( _members );

    // Buttons
    QHBoxLayout* lay6 = new QHBoxLayout( lay1, 6 );
    lay6->addStretch(1);

    QPushButton* add = new QPushButton( i18n("Add Group..." ), top );
    lay6->addWidget( add );
    _rename = new QPushButton( i18n( "Rename Group..."), top );
    lay6->addWidget( _rename );
    _del = new QPushButton( i18n("Delete Group" ), top );
    lay6->addWidget( _del );

    // Setup the actions
    _memberMap = ImageDB::instance()->memberMap();
    connect( _category, SIGNAL( activated( const QString& ) ), this, SLOT( slotCategoryChanged( const QString& ) ) );
    connect( _groups, SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( slotGroupSelected( QListBoxItem* ) ) );
    connect( _rename, SIGNAL( clicked() ), this, SLOT( slotRenameGroup() ) );
    connect( add, SIGNAL( clicked() ), this, SLOT( slotAddGroup() ) );
    connect( _del, SIGNAL( clicked() ), this, SLOT( slotDelGroup() ) );

    _members->setSelectionMode( QListBox::Multi );
}

/**
   When the user selects a new category from the combo box then this method is called
   Its purpose is too fill the groups and members listboxes.
*/
void OptionsDialog::slotCategoryChanged( const QString& text )
{
    slotCategoryChanged( ImageDB::instance()->categoryCollection()->nameForText(text), true );
}

void OptionsDialog::slotCategoryChanged( const QString& name, bool saveGroups )
{
    if ( saveGroups ) {
        // We do not want to save groups when renaming categories
        saveOldGroup();
    }

    _groups->clear();
    _currentCategory = name;
    QStringList groupList = _memberMap.groups( name );
    _groups->insertStringList( groupList );

    _members->clear();
    QStringList list = ImageDB::instance()->categoryCollection()->categoryForName(name)->items();
    list += _memberMap.groups( name );
    QStringList uniq;
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( !uniq.contains(*it) )
            uniq << *it;
    }

    uniq.sort();
    _members->insertStringList( uniq );

    if ( !groupList.isEmpty() ) {
        _groups->setSelected( 0, true );
        selectMembers( _groups->text(0) );
    }
    else
        _currentGroup = QString::null;

    setButtonStates();
}

void OptionsDialog::slotGroupSelected( QListBoxItem* item )
{
    saveOldGroup();
    if ( item )
        selectMembers( item->text() );
}

void OptionsDialog::slotAddGroup()
{
    bool ok;
    QString text = KInputDialog::getText( i18n( "New Group" ), i18n("Group name:"), QString::null, &ok );
    if ( ok ) {
        saveOldGroup();
        QListBoxItem* item = new QListBoxText( _groups, text );
        _groups->setCurrentItem( item );
        selectMembers( text );
        ImageDB::instance()->categoryCollection()->categoryForName( _currentCategory )->addItem( text );
        _memberMap.setMembers(_currentCategory, text, QStringList() );
        slotCategoryChanged( _currentCategory, false );
    }
}

void OptionsDialog::slotRenameGroup()
{
    Q_ASSERT( !_currentGroup.isNull() );
    bool ok;
    QListBoxItem* item = _groups->item( _groups->currentItem() );
    QString currentValue = item->text();
    QString text = KInputDialog::getText( i18n( "New Group" ), i18n("Group name:"), currentValue, &ok );
    if ( ok ) {
        saveOldGroup();
        _memberMap.renameGroup( _currentCategory, currentValue, text );
        ImageDB::instance()->categoryCollection()->categoryForName( _currentCategory )->renameItem( currentValue, text );
        slotCategoryChanged( _currentCategory, false );
    }
}

void OptionsDialog::slotDelGroup()
{
    Q_ASSERT( !_currentGroup.isNull() );
    int res = KMessageBox::warningContinueCancel( this, i18n( "Really delete group %1?" ).arg( _currentGroup ),i18n("Delete Group"),KGuiItem(i18n("&Delete"),QString::fromLatin1("editdelete")) );
    if ( res == KMessageBox::Cancel )
        return;

    saveOldGroup();

    QListBoxItem* item = _groups->findItem( _currentGroup );
    delete item;

    _memberMap.deleteGroup( _currentCategory, _currentGroup );
    ImageDB::instance()->categoryCollection()->categoryForName( _currentCategory )->removeItem( _currentGroup );
    _currentGroup = _groups->text(0);
    slotCategoryChanged( _currentCategory, false );
    selectMembers( _currentGroup );
    setButtonStates();
}

void OptionsDialog::saveOldGroup()
{
    if ( _currentCategory.isNull() || _currentGroup.isNull() )
        return;

    QStringList list;
    for( QListBoxItem* item = _members->firstItem(); item; item = item->next() ) {
        if ( item->isSelected() )
            list << item->text();
    }

    _memberMap.setMembers(_currentCategory, _currentGroup, list);
}

void OptionsDialog::selectMembers( const QString& group )
{
    _currentGroup = group;
    QStringList list = _memberMap.members(_currentCategory,group, false );
    for( QListBoxItem* item = _members->firstItem(); item; item = item->next() ) {
        _members->setSelected( item, list.contains( item->text() ) );
    }
    setButtonStates();
}


int OptionsDialog::exec()
{
    slotCategoryChanged( _currentCategory, false );
    return KDialogBase::exec();
}

void OptionsDialog::setButtonStates()
{
    bool b = !_currentGroup.isNull();
    _rename->setEnabled( b );
    _del->setEnabled( b );
}


void OptionsDialog::slotPageChange()
{
    _category->clear();
    QValueList<CategoryPtr> categories = ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if ( !(*it)->isSpecialCategory() )
            _category->insertItem( (*it)->text() );
    }

    slotCategoryChanged( _category->currentText() );
}






void OptionsDialog::createViewerPage()
{
    QWidget* top = addPage( i18n("Viewer" ), i18n("Viewer" ),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "viewmag" ),
                                                             KIcon::Desktop, 32 ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    _slideShowSetup = new ViewerSizeConfig( i18n( "Running Slide Show From Thumbnail View" ), top, "_slideShowSetup" );
    lay1->addWidget( _slideShowSetup );

    _viewImageSetup = new ViewerSizeConfig( i18n( "Viewing Images From Thumbnail View" ), top, "_viewImageSetup" );
    lay1->addWidget( _viewImageSetup );

    QGridLayout* glay = new QGridLayout( lay1, 2, 2, 6 );

    QLabel* label = new QLabel( i18n("Slideshow interval:" ), top );
    glay->addWidget( label, 0, 0 );

    _slideShowInterval = new QSpinBox( 1, INT_MAX, 1, top );
    glay->addWidget( _slideShowInterval, 0, 1 );
    _slideShowInterval->setSuffix( i18n( " sec" ) );

    label = new QLabel( i18n("Image cache:"), top );
    glay->addWidget( label, 1, 0 );

    _cacheSize = new QSpinBox( 0, 2000, 10, top, "_cacheSize" );
    _cacheSize->setSuffix( i18n(" Mbytes") );
    glay->addWidget( _cacheSize, 1, 1 );
}


void OptionsDialog::createPluginPage()
{
#ifdef HASKIPI
    MainView::theMainView()->loadPlugins();
    QWidget* top = addPage( i18n("Plugins" ), i18n("Plugins" ),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "share" ),
                                                             KIcon::Desktop, 32 ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    QLabel* label = new QLabel( i18n("Choose Plugins to load:"), top );
    lay1->addWidget( label );

    _pluginConfig = KIPI::PluginLoader::instance()->configWidget( top );
    lay1->addWidget( _pluginConfig );

    _delayLoadingPlugins = new QCheckBox( i18n("Delay loading plug-ins till plug-in menu is opened"), top );
    lay1->addWidget( _delayLoadingPlugins );
#endif
}

void OptionsDialog::createEXIFPage()
{
#ifdef HASEXIV2
    QWidget* top = addPage( i18n("EXIF Information" ), i18n("Exif Information" ),
                            KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "contents" ),
                                                             KIcon::Desktop, 32 ) );
    QHBoxLayout* lay1 = new QHBoxLayout( top, 6 );

    _exifForViewer = new Exif::TreeView( i18n( "EXIF info to show in the Viewer" ), top );
    lay1->addWidget( _exifForViewer );

    _exifForDialog = new Exif::TreeView( i18n("EXIF info to show in the EXIF dialog"), top );
    lay1->addWidget( _exifForDialog );
#endif
}

