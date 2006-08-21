#ifndef SETTINGS_CATEGORYITEM_H
#define SETTINGS_CATEGORYITEM_H

#include <qlistbox.h>
#include <DB/Category.h>

namespace Settings
{
class CategoryItem :public QListBoxText
{
public:
    CategoryItem( const QString& category, const QString& text, const QString& icon,
                  DB::Category::ViewSize size, DB::Category::ViewType type, int thumbnailSize, QListBox* parent );
    void setLabel( const QString& label );

    QString _categoryOrig, _textOrig, _iconOrig;
    QString _text, _icon;
    DB::Category::ViewSize _size, _sizeOrig;
    DB::Category::ViewType _type, _typeOrig;
    int _thumbnailSize, _thumbnailSizeOrig;
};

}


#endif /* SETTINGS_CATEGORYITEM_H */

