#ifndef SQLCATEGORYCOLLECTION_H
#define SQLCATEGORYCOLLECTION_H

#include "DB/CategoryCollection.h"
#include "Connection.h"
#include "QueryHelper.h"

namespace SQLDB {
    class SQLCategoryCollection :public DB::CategoryCollection
    {
        Q_OBJECT

    public:
        explicit SQLCategoryCollection(Connection& connection);

        virtual DB::CategoryPtr categoryForName(const QString& name) const;
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<DB::CategoryPtr> categories() const;
        virtual void addCategory(const QString& text, const QString& icon,
                                 DB::Category::ViewType type,
                                 int thumbnailSize, bool show );

    protected:
        Connection* _connection;
        QueryHelper _qh;
    };
}

#endif /* SQLCATEGORYCOLLECTION_H */

