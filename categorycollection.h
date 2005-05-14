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
    CategoryCollection();
    Category* categoryForName( const QString& name );
    void addCategory( Category* );
    QStringList categoryNames();
    void removeCategory( const QString& name );
    void rename( const QString& oldName, const QString& newName );
    const QValueList<Category*>& categories() const;
    QString nameForText( const QString& text );

signals:
    void categoryCollectionChanged();
    void itemRenamed( Category* category, const QString& oldName, const QString& newName );
    void itemRemoved( Category* category, const QString& name );

protected slots:
    void itemRenamed( const QString& oldName, const QString& newName );
    void itemRemoved( const QString& item );

private:
    QValueList<Category*> _categories;
};


#endif /* CATEGORYCOLLECTION_H */

