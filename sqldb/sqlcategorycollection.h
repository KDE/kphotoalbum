#ifndef SQLCATEGORYCOLLECTION_H
#define SQLCATEGORYCOLLECTION_H

#include <categorycollection.h>

namespace SQLDB {
    class SQLCategoryCollection :public CategoryCollection
    {
        Q_OBJECT

    public:
        virtual CategoryPtr categoryForName( const QString& name ) const;
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<CategoryPtr> categories() const;
        virtual void addCategory( const QString& text, const QString& icon, Category::ViewSize size, Category::ViewType type, bool show = true );
    };
}

#endif /* SQLCATEGORYCOLLECTION_H */

