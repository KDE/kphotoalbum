#ifndef SQLCATEGORYCOLLECTION_H
#define SQLCATEGORYCOLLECTION_H

#include "DB/CategoryCollection.h"

namespace SQLDB {
    class SQLCategoryCollection :public DB::CategoryCollection
    {
        Q_OBJECT

    public:
        virtual DB::CategoryPtr categoryForName( const QString& name ) const;
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<DB::CategoryPtr> categories() const;
        virtual void addCategory( const QString& text, const QString& icon, DB::Category::ViewSize size, DB::Category::ViewType type, bool show = true );
    };
}

#endif /* SQLCATEGORYCOLLECTION_H */

