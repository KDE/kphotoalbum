#ifndef CATEGORYCOLLECTION_H
#define CATEGORYCOLLECTION_H

#include <qvaluelist.h>
#include "options.h"

class Category;

/**
   This class is the collection of categories. It is the basic anchor point to categories.
*/

class CategoryCollection :public QObject
{
    Q_OBJECT

public:
    static CategoryCollection* instance();
    Category* categoryForName( const QString& name );
    void addCategory( Category* );
    QStringList categoryNames();
    void removeCategory( const QString& name );
    void rename( const QString& oldName, const QString& newName );
    const QValueList<Category*>& categories() const;
    QString nameForText( const QString& text );

signals:
    void categoryCollectionChanged();

private:
    static CategoryCollection* _instance;
    CategoryCollection();

    QValueList<Category*> _categories;
};


#endif /* CATEGORYCOLLECTION_H */

