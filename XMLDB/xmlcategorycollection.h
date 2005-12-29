#ifndef XMLCATEGORYCOLLECTION_H
#define XMLCATEGORYCOLLECTION_H

#include <categorycollection.h>

namespace XMLDB {

    class XMLCategoryCollection :public CategoryCollection
    {
        Q_OBJECT

    public:
        virtual CategoryPtr categoryForName( const QString& name ) const;
        void addCategory( Category* );
        virtual QStringList categoryNames() const;
        virtual void removeCategory( const QString& name );
        virtual void rename( const QString& oldName, const QString& newName );
        virtual QValueList<CategoryPtr> categories() const;
        virtual void addCategory( const QString& text, const QString& icon, Category::ViewSize size, Category::ViewType type, bool show = true );

        void initIdMap();

    private:
        QValueList<CategoryPtr> _categories;
    };
}

#endif /* XMLCATEGORYCOLLECTION_H */

