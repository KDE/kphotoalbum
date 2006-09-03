#ifndef SETTINGS_CATEGORYITEM_H
#define SETTINGS_CATEGORYITEM_H

#include <qlistbox.h>
#include <DB/Category.h>
namespace DB { class MemberMap; }

namespace Settings
{
class CategoryItem :public QListBoxText
{
public:
    CategoryItem( const QString& category, const QString& text, const QString& icon,
                  DB::Category::ViewType type, int thumbnailSize, QListBox* parent );
    void setLabel( const QString& label );
    void submit( DB::MemberMap* memberMap );
    void removeFromDatabase();

    QString text() const;
    QString icon() const;
    int thumbnailSize() const;
    DB::Category::ViewType viewType() const;

    void setIcon( const QString& icon );
    void setThumbnailSize( int size );
    void setViewType( DB::Category::ViewType type );

private:
    QString _categoryOrig, _textOrig, _iconOrig;
    QString _category, _text, _icon;
    DB::Category::ViewType _type, _typeOrig;
    int _thumbnailSize, _thumbnailSizeOrig;
};

}


#endif /* SETTINGS_CATEGORYITEM_H */

