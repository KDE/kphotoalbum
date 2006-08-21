#include "CategoryItem.h"

Settings::CategoryItem::CategoryItem( const QString& category, const QString& text, const QString& icon,
                                      DB::Category::ViewSize size, DB::Category::ViewType type, int thumbnailSize, QListBox* parent )
    :QListBoxText( parent, text ),
     _categoryOrig( category ), _textOrig( text ), _iconOrig( icon ),
     _text( text ), _icon( icon ), _size( size ), _sizeOrig( size ), _type( type ), _typeOrig( type ),
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


