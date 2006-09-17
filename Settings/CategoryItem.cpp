#include "CategoryItem.h"
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"
#include <qdir.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "MainWindow/Window.h"

Settings::CategoryItem::CategoryItem( const QString& category, const QString& text, const QString& icon,
                                      DB::Category::ViewType type, int thumbnailSize, QListBox* parent )
    :QListBoxText( parent, text ),
     _categoryOrig( category ), _textOrig( text ), _iconOrig( icon ),
     _category( category ), _text( text ), _icon( icon ), _type( type ), _typeOrig( type ),
     _thumbnailSize( thumbnailSize ), _thumbnailSizeOrig( thumbnailSize )
{
}

void Settings::CategoryItem::setLabel( const QString& label )
{
    setText( label );
    _text = label;

    // unfortunately setText do not call updateItem, so we need to force it with this hack.
    bool b = listBox()->isSelected( this );
    listBox()->setSelected( this, !b );
    listBox()->setSelected( this, b );
}

void Settings::CategoryItem::submit( DB::MemberMap* memberMap )
{
    if ( _categoryOrig.isNull() ) {
        // New Item
        DB::ImageDB::instance()->categoryCollection()->addCategory( _text, _icon, _type, _thumbnailSize, true );
    }
    else {
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( _categoryOrig );
        if ( _text != _textOrig )
            renameCategory( memberMap );

        if ( _icon != _iconOrig )
            category->setIconName( _icon );

        if ( _type != _typeOrig )
            category->setViewType( _type );

        if ( _thumbnailSize != _thumbnailSizeOrig )
            category->setThumbnailSize( _thumbnailSize );
    }

    _categoryOrig = _category;
    _iconOrig = _icon;
    _typeOrig = _typeOrig;
    _thumbnailSizeOrig = _thumbnailSize;
    _textOrig = _text;
}

void Settings::CategoryItem::removeFromDatabase()
{
    if ( !_categoryOrig.isNull() ) {
        // the database knows about the item.
        DB::ImageDB::instance()->categoryCollection()->removeCategory( _categoryOrig );
    }
}

QString Settings::CategoryItem::text() const
{
    return _text;
}

QString Settings::CategoryItem::icon() const
{
    return _icon;
}

int Settings::CategoryItem::thumbnailSize() const
{
    return _thumbnailSize;
}

DB::Category::ViewType Settings::CategoryItem::viewType() const
{
    return _type;
}

void Settings::CategoryItem::setIcon( const QString& icon )
{
    _icon = icon;
}

void Settings::CategoryItem::setThumbnailSize( int size )
{
    _thumbnailSize = size;
}

void Settings::CategoryItem::setViewType( DB::Category::ViewType type )
{
    _type = type;
}

void Settings::CategoryItem::renameCategory( DB::MemberMap* memberMap )
{
    QString txt = i18n("<p>Due to a shortcomming in KPhotoAlbum, you need to save your database after renaming categories "
                       "otherwise all the filenames of the category thumbnails will be wrong, and thus lost.</p>"
                       "<p>So either press cancel now (and it will not be renamed), or press continue, and as your next "
                       "step save the database.</p>" );


    if ( KMessageBox::warningContinueCancel( MainWindow::Window::theMainWindow(), txt ) == QMessageBox::Cancel )
        return;

    QDir dir( QString::fromLatin1("%1/CategoryImages" ).arg( Settings::SettingsData::instance()->imageDirectory() ) );
    const QStringList files = dir.entryList( QString::fromLatin1("%1*" ).arg( _categoryOrig ) );
    for( QStringList::ConstIterator fileNameIt = files.begin(); fileNameIt != files.end(); ++fileNameIt ) {
        QString newName = _text + (*fileNameIt).mid( _categoryOrig.length() );
        dir.rename( *fileNameIt, newName );
    }

    Settings::SettingsData* settings = Settings::SettingsData::instance();
    DB::ImageSearchInfo info = settings->currentLock();
    const bool exclude = settings->lockExcludes();
    info.renameCategory( _categoryOrig, _text );
    settings->setCurrentLock( info, exclude );

    DB::ImageDB::instance()->categoryCollection()->rename(  _categoryOrig, _text );
    memberMap->renameCategory(  _categoryOrig, _text );
    _categoryOrig =_text;
}


