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
#include "Category.h"
#include <kmessagebox.h>
#include <QDir>
#include <QFileInfo>
#include <Settings/SettingsData.h>
#include <QPixmapCache>
#include <kiconloader.h>
#include <klocale.h>
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"
#include "CategoryItem.h"
#include <QPixmap>
#include <KIcon>
#include <Utilities/Util.h>

using Utilities::StringSet;

QPixmap DB::Category::icon( int size ) const
{
    QPixmap pixmap = KIconLoader::global()->loadIcon( iconName(), KIconLoader::Desktop, size, KIconLoader::DefaultState, QStringList(), 0L, true);
    DB::Category* This = const_cast<DB::Category*>(this);
    if ( pixmap.isNull() ) {
        This->blockSignals(true);
        This->setIconName(defaultIconName());
        This->blockSignals(false);
        pixmap = KIcon(iconName()).pixmap(size);
    }
    return pixmap;
}


/**
   If one person from say Denmark sends a database to a person from say germany, then the title of
   People, Places, and Events will still be translated correct, when this function is used.
*/
QString DB::Category::text() const
{
    if (standardCategories().contains( name() ) )
        return standardCategories()[name()];
    else
        return name();
}

QStringList DB::Category::itemsInclCategories() const
{
    // values including member groups

    QStringList items = this->items();

    // add the groups to the list too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = DB::ImageDB::instance()->memberMap().groups( name() );
    for( QStringList::ConstIterator it = groups.constBegin(); it != groups.constEnd(); ++it ) {
        if ( ! items.contains(  *it ) )
            items << *it ;
    };

    return items;
}

DB::CategoryItem* createItem( const QString& categoryName, const QString& itemName, StringSet handledItems,
                              QMap<QString,DB::CategoryItem*>& categoryItems,
                              QMap<QString, DB::CategoryItem*>& potentialToplevelItems )
{
    handledItems.insert( itemName );
    DB::CategoryItem* result = new DB::CategoryItem( itemName );

    const QStringList members = DB::ImageDB::instance()->memberMap().members( categoryName, itemName, false );
    for( QStringList::ConstIterator memberIt = members.constBegin(); memberIt != members.constEnd(); ++memberIt ) {
        if ( !handledItems.contains( *memberIt ) ) {
            DB::CategoryItem* child;
            if ( categoryItems.contains( *memberIt ) )
                child = categoryItems[*memberIt]->clone();
            else
                child = createItem( categoryName, *memberIt, handledItems, categoryItems, potentialToplevelItems );

            potentialToplevelItems.remove( *memberIt );
            result->_subcategories.append( child );
        }
    }

    categoryItems.insert( itemName, result );
    return result;
}

DB::CategoryItemPtr DB::Category::itemsCategories() const
{
    const MemberMap& map = ImageDB::instance()->memberMap();
    const QStringList groups = map.groups( name() );

    QMap<QString, DB::CategoryItem*> categoryItems;
    QMap<QString, DB::CategoryItem*> potentialToplevelItems;

    for( QStringList::ConstIterator groupIt = groups.constBegin(); groupIt != groups.constEnd(); ++groupIt ) {
        if ( !categoryItems.contains( *groupIt ) ) {
            StringSet handledItems;
            DB::CategoryItem* child = createItem( name(), *groupIt, handledItems, categoryItems, potentialToplevelItems );
            potentialToplevelItems.insert( *groupIt, child );
        }
    }


    CategoryItem* result = new CategoryItem( QString::fromLatin1("top"), true );
    for( QMap<QString,DB::CategoryItem*>::ConstIterator toplevelIt = potentialToplevelItems.constBegin(); toplevelIt != potentialToplevelItems.constEnd(); ++toplevelIt ) {
        result->_subcategories.append( *toplevelIt );
    }

    // Add items not found yet.
    QStringList elms = items();
    for( QStringList::ConstIterator elmIt = elms.constBegin(); elmIt != elms.constEnd(); ++elmIt ) {
        if ( !categoryItems.contains( *elmIt ) )
            result->_subcategories.append( new DB::CategoryItem( *elmIt ) );
    }

    return CategoryItemPtr( result );
}

QMap<QString,QString> DB::Category::standardCategories()
{
    static QMap<QString,QString> map;
    if ( map.isEmpty() ) {
        map.insert( QString::fromLatin1( "People" ), i18n("People") );
        map.insert( QString::fromLatin1( "Places" ), i18n("Places") );
        map.insert( QString::fromLatin1( "Events" ),  i18n("Events") );
        map.insert( QString::fromLatin1( "Folder" ),  i18n("Folder") );
        map.insert( QString::fromLatin1( "Tokens" ),  i18n("Tokens") );
        map.insert( QString::fromLatin1( "Media Type" ),  i18n("Media Type") );

        // Needed for compatibility with index.xml files from older versions of KPA.
        map.insert( QString::fromLatin1( "Persons" ), i18n("People") );
        map.insert( QString::fromLatin1( "Locations" ), i18n("Places") );
        map.insert( QString::fromLatin1( "Keywords" ),  i18n("Keywords") );
    }
    return map;
}

QString DB::Category::defaultIconName() const
{
    const QString nm = name().toLower();
    if ( nm == QString::fromLatin1("people") ) return QString::fromLatin1("system-users");
    if ( nm == QString::fromLatin1("places") || nm == QString::fromLatin1("locations") ) return QString::fromLatin1("network-workgroup");
    if ( nm == QString::fromLatin1("events") || nm == QString::fromLatin1("keywords") ) return QString::fromLatin1("dialog-password");
    if ( nm == QString::fromLatin1("tokens") ) return QString::fromLatin1("preferences-other");
    if ( nm == QString::fromLatin1("folder") ) return QString::fromLatin1("folder");
    if ( nm == QString::fromLatin1("media type") ) return QString::fromLatin1("video");
    return QString();
}

QPixmap DB::Category::categoryImage( const QString& category, QString member, int width, int height ) const
{
    QString fileName = fileForCategoryImage( category, member );
    QString key = QString::fromLatin1( "%1-%2" ).arg(width).arg(fileName);
    QPixmap res;
    if ( QPixmapCache::find( key, res ) )
        return res;

    QImage img;
    bool ok = img.load( fileName, "JPEG" );
    if ( ! ok ) {
        if ( DB::ImageDB::instance()->memberMap().isGroup( category, member ) )
            img = KIconLoader::global()->loadIcon( QString::fromLatin1( "kuser" ), KIconLoader::Desktop, qMax(width,height) ).toImage();
        else
            img = icon( qMax(width,height) ).toImage();
    }
    res = QPixmap::fromImage( Utilities::scaleImage(img, width, height, Qt::KeepAspectRatio) );

    QPixmapCache::insert( key, res );
    return res;
}



void DB::Category::setCategoryImage( const QString& category, QString member, const QImage& image )
{
    QString dir = Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("CategoryImages" );
    QFileInfo fi( dir );
    bool ok;
    if ( !fi.exists() ) {
        bool ok = QDir().mkdir( dir );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to create directory '%1'.", dir ), i18n("Unable to Create Directory") );
            return;
        }
    }
    QString fileName = fileForCategoryImage( category, member );
    ok = image.save( fileName, "JPEG" );
    if ( !ok ) {
        KMessageBox::error( 0, i18n("Error when saving image '%1'.",fileName), i18n("Error Saving Image") );
        return;
    }

    QPixmapCache::clear();
}

QString DB::Category::fileForCategoryImage( const QString& category, QString member ) const
{
    QString dir = Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("CategoryImages" );
    member.replace( QChar::fromLatin1(' '), QChar::fromLatin1('_') );
    member.replace( QChar::fromLatin1('/'), QChar::fromLatin1('_') );
    QString fileName = dir + QString::fromLatin1("/%1-%2.jpg").arg( category ).arg( member );
    return fileName;
}


#include "Category.moc"
