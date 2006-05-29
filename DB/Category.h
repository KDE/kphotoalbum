#ifndef CATEGORY_H
#define CATEGORY_H

#include <qstring.h>
#include <qpixmap.h>
#include <qobject.h>
#include <ksharedptr.h>

namespace DB
{


/**
   This class stores information about categories (Persons/Locations/Keywords)
*/
class Category :public QObject, public KShared
{
    Q_OBJECT

public:
    enum ViewSize { Small, Large };
    enum ViewType { ListView, IconView };

    virtual QString name() const = 0;
    virtual void setName( const QString& name ) = 0;

    virtual QString text() const;

    virtual QString iconName() const = 0;
    virtual void setIconName( const QString& name ) = 0;
    virtual QPixmap icon( int size = 22 ) const;

    virtual ViewSize viewSize() const = 0;
    virtual void setViewSize( ViewSize size ) = 0;

    virtual void setViewType( ViewType type ) = 0;
    virtual ViewType viewType() const = 0;

    virtual void setDoShow( bool b ) = 0;
    virtual bool doShow() const = 0;

    virtual void setSpecialCategory( bool b ) = 0;
    virtual bool isSpecialCategory() const = 0;

    virtual void setItems( const QStringList& items ) = 0;
    virtual void removeItem( const QString& item ) = 0;
    virtual void renameItem( const QString& oldValue, const QString& newValue ) = 0;
    virtual void addItem( const QString& item ) = 0;
    virtual QStringList items() const = 0;
    virtual QStringList itemsInclGroups() const;

signals:
    void changed();
    void itemRenamed( const QString& oldName, const QString& newName );
    void itemRemoved( const QString& name );
};

}

#endif /* CATEGORY_H */

