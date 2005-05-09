#ifndef CATEGORY_H
#define CATEGORY_H

#include <qstring.h>
#include <qpixmap.h>
#include <qobject.h>


/**
   This class stores information about categories (Persons/Locations/Keywords)
*/
class Category :public QObject
{
    Q_OBJECT

public:
    enum ViewSize { Small, Large };
    enum ViewType { ListView, IconView };

    Category( const QString& name, const QString& icon, ViewSize size, ViewType type, bool show = true );

    QString name() const;
    void setName( const QString& name );

    QString text() const;

    QString iconName() const;
    void setIconName( const QString& name );
    QPixmap icon( int size = 22 );

    ViewSize viewSize() const;
    void setViewSize( ViewSize size );

    void setViewType( ViewType type );
    ViewType viewType() const;

    void setDoShow( bool b );
    bool doShow() const;

    void setSpecialCategory( bool b );
    bool isSpecialCategory() const;

    void setItems( const QStringList& items );
    void removeItem( const QString& item );
    void renameItem( const QString& oldValue, const QString& newValue );
    void addItem( const QString& item );
    QStringList items() const;
    QStringList itemsInclGroups() const;

signals:
    void changed();
    void itemRenamed( const QString& oldName, const QString& newName );
    void itemRemoved( const QString& name );

private:
    QString _name;
    QString _icon;
    bool _show;
    ViewSize _size;
    ViewType _type;

    bool _isSpecial;
    QStringList _items;
};

#endif /* CATEGORY_H */

