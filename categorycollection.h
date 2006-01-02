#ifndef CATEGORYCOLLECTION_H
#define CATEGORYCOLLECTION_H

#include <qvaluelist.h>
#include "options.h"
#include <ksharedptr.h>
#include "category.h"

class Category;

/**
   \class CategoryCollection
   This class is the collection of categories. It is the basic anchor point to categories.
*/

typedef KSharedPtr<Category> CategoryPtr;
class CategoryCollection :public QObject
{
    Q_OBJECT

public:
    virtual CategoryPtr categoryForName( const QString& name ) const = 0;
    virtual QStringList categoryNames() const = 0;
    virtual void removeCategory( const QString& name ) = 0;
    virtual void rename( const QString& oldName, const QString& newName ) = 0;
    virtual QValueList<CategoryPtr> categories() const = 0;
    virtual void addCategory( const QString& text, const QString& icon, Category::ViewSize size, Category::ViewType type, bool show = true ) = 0;

    QString nameForText( const QString& text );

signals:
    void categoryCollectionChanged();
    void itemRenamed( Category* category, const QString& oldName, const QString& newName );
    void itemRemoved( Category* category, const QString& name );

protected slots:
    void itemRenamed( const QString& oldName, const QString& newName );
    void itemRemoved( const QString& item );
};


#endif /* CATEGORYCOLLECTION_H */

