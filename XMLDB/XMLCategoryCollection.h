#ifndef XMLCATEGORYCOLLECTION_H
#define XMLCATEGORYCOLLECTION_H

#include "DB/CategoryCollection.h"

namespace XMLDB {

    class XMLCategoryCollection :public DB::CategoryCollection
    {
        Q_OBJECT

    public:
        virtual DB::CategoryPtr categoryForName( const QString& name ) const;
        void addCategory( DB::Category* );
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<DB::CategoryPtr> categories() const;
        virtual void addCategory( const QString& text, const QString& icon, DB::Category::ViewSize size, DB::Category::ViewType type, bool show = true );

        void initIdMap();

    private:
        QValueList<DB::CategoryPtr> _categories;
    };
}

#endif /* XMLCATEGORYCOLLECTION_H */

