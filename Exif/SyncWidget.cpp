/* Copyright (C) 2007 Jan Kundrát <jkt@gentoo.org>

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
#include <klocale.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmap.h>
#include <qwhatsthis.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include "Exif/SyncWidget.h"

namespace Exif {

SyncWidget::SyncWidget( const QString& title, QWidget* parent, const QValueList<Syncable::Kind>& items, const char* name )
    :QHBox( parent, name )
{
    QMap<Syncable::Kind,QString> _fieldName, _visibleName;
    QMap<Syncable::Kind,Syncable::Header> _header;
    Syncable::fillTranslationTables( _fieldName, _visibleName, _header);

    setSpacing( 6 );
    _list = new QListView( this, name );
    _list->setSorting( -1 );
    _list->addColumn( title );

    // records in QListView has to be created in reverse order...
    QValueList<Syncable::Kind>::const_iterator it = items.end();
    if ( it != items.begin() )
        do {
            --it;
            if ( _header.contains( *it ) ) {
                _items.append( *it );
                new QListViewItem( _list, _visibleName[*it], QString::number( static_cast<int>( *it ) ) );
            }
        } while ( it != items.begin() );

    QVBox* vbox = new QVBox( this );
    QWidget* spacer = new QWidget( vbox );
    _upBut = new QPushButton( vbox );
    _upBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "up" ), KIcon::Desktop, 22 ) );
    _downBut = new QPushButton( vbox );
    _downBut->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "down" ), KIcon::Desktop, 22 ) );
    spacer = new QWidget( vbox );

    QWhatsThis::add( this, i18n("<p>Use the \"Up\" and \"Down\" buttons to change the order of various fields. "
                "Items are processed from the top of this list and the iteration stops when it hits the "
                "<code>-- stop --</code> field.</p>") );

    slotHandleDisabling();

    connect( _upBut, SIGNAL( clicked() ), this, SLOT( slotMoveSelectedUp() ) );
    connect( _downBut, SIGNAL( clicked() ), this, SLOT( slotMoveSelectedDown() ) );
    connect( _list, SIGNAL( selectionChanged() ), this, SLOT( slotHandleDisabling() ) );
}

void SyncWidget::slotMoveSelectedDown()
{
    QListViewItem* item = _list->selectedItem();
    if ( !item )
        return;
    QListViewItem* other = item->itemBelow();
    if ( other ) {
        Syncable::Kind kind = static_cast<Syncable::Kind>( item->text( 1 ).toInt() );
        QValueList<Syncable::Kind>::iterator current, next;
        next = current = _items.find( kind );
        Q_ASSERT( current != _items.end() );
        --next;
        *current = *next;
        *next = kind;

        item->moveItem( other );

        slotHandleDisabling();
    }
}

void SyncWidget::slotMoveSelectedUp()
{
    QListViewItem* item = _list->selectedItem();
    if ( !item )
        return;
    QListViewItem* other = item->itemAbove();
    if ( other ) {
        // remember, _items are reversed
        Syncable::Kind kind = static_cast<Syncable::Kind>( item->text( 1 ).toInt() );
        QValueList<Syncable::Kind>::iterator current, next;
        next = current = _items.find( kind );
        ++next;
        Q_ASSERT( ( current != _items.end() ) && ( next != _items.end() ) );
        *current = *next;
        *next = kind;

        other->moveItem( item );

        slotHandleDisabling();
    }
}

void SyncWidget::slotHandleDisabling()
{
    _upBut->setDisabled( true );
    _downBut->setDisabled( true );

    QListViewItem* item = _list->selectedItem();
    if ( !item )
        return;
    QListViewItem* other = item->itemAbove();
    if ( other )
        _upBut->setEnabled( true );
    other = item->itemBelow();
    if ( other )
        _downBut->setEnabled( true );
}

QValueList<Syncable::Kind> SyncWidget::items() const
{
    // _items are reversed, so we have to reverse them again here
    QValueList<Syncable::Kind> items;
    QValueList<Syncable::Kind>::const_iterator it = _items.end();
    if ( it != _items.begin() )
        do {
            --it;
            items.append( *it );
        } while ( it != _items.begin() );
    return items;
}


namespace Syncable {

void fillTranslationTables( QMap<Kind,QString>& _fieldName, QMap<Kind,QString>& _visibleName, QMap<Kind,Header>& _header)
{
#define II(X,HEADER,FIELD,VISIBLE) \
     _header[X] = HEADER; \
     _fieldName[X] = #FIELD; \
     _visibleName[X] = i18n(VISIBLE);
#define I(X,HEADER,FIELD) \
     _header[X] = HEADER; \
     _fieldName[X] = #FIELD; \
     _visibleName[X] = QString::fromAscii(#FIELD);


    II(STOP, NONE, STOP, "-- stop --");
    II(JPEG_COMMENT, JPEG, JPEG.Comment, "JPEG Comment");
    II(EXIF_ORIENTATION, EXIF, Exif.Image.Orientation, "EXIF Image Orientation");
    II(EXIF_DESCRIPTION, EXIF, Exif.Image.ImageDescription, "EXIF Image Description");
    II(EXIF_USER_COMMENT, EXIF, Exif.Photo.UserComment, "EXIF User Comment");

    I(EXIF_XPTITLE, EXIF, Exif.Image.XPTitle);
    I(EXIF_XPCOMMENT, EXIF, Exif.Image.XPComment);
    I(EXIF_XPKEYWORDS, EXIF, Exif.Image.XPKeywords);
    I(EXIF_XPSUBJECT, EXIF, Exif.Image.XPSubject);

    II(IPTC_HEADLINE, IPTC, Iptc.Application2.Headline, "IPTC Headline");
    II(IPTC_CAPTION, IPTC, Iptc.Application2.Caption, "IPTC Caption");
    I(IPTC_OBJECT_NAME, IPTC, Iptc.Application2.ObjectName);
    II(IPTC_SUBJECT, IPTC, Iptc.Application2.Subject, "IPTC Subject");

    II(IPTC_SUPP_CAT, IPTC, Iptc.Application2.SuppCategory, "IPTC Supplemental Categories");
    II(IPTC_KEYWORDS, IPTC, Iptc.Application2.Keywords, "IPTC Keywords");

    I(IPTC_LOCATION_CODE, IPTC, Iptc.Application2.LocationCode);
    I(IPTC_LOCATION_NAME, IPTC, Iptc.Application2.LocationName);
    I(IPTC_CITY, IPTC, Iptc.Application2.City);
    I(IPTC_SUB_LOCATION, IPTC, Iptc.Application2.SubLocation);
    I(IPTC_PROVINCE_STATE, IPTC, Iptc.Application2.ProvinceState);
    I(IPTC_COUNTRY_CODE, IPTC, Iptc.Application2.CountryCode);
    I(IPTC_COUNTRY_NAME, IPTC, Iptc.Application2.CountryName);

    II(FILE_CTIME, FILE, File.CTime, "File creation time");
    II(FILE_MTIME, FILE, File.MTime, "File last modification time");
    II(EXIF_DATETIME, EXIF, Exif.Image.DateTime, "EXIF Date");
    I(EXIF_DATETIME_ORIGINAL, EXIF, Exif.Photo.DateTimeOriginal);
    I(EXIF_DATETIME_DIGITIZED, EXIF, Exif.Photo.DateTimeDigitized);

#undef I
#undef II
}

}

}

#include "SyncWidget.moc"
