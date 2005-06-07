#ifndef SQLCATEGORY_H
#define SQLCATEGORY_H
#include <category.h>

namespace SQLDB {
    class SQLCategory :public Category
    {
    public:
        SQLCategory( const QString& category );
        virtual QString name() const;
        virtual void setName( const QString& name );

        virtual QString iconName() const;
        virtual void setIconName( const QString& name );

        virtual ViewSize viewSize() const;
        virtual void setViewSize( ViewSize size );

        virtual void setViewType( ViewType type );
        virtual ViewType viewType() const;

        virtual void setDoShow( bool b );
        virtual bool doShow() const;

        virtual void setSpecialCategory( bool b );
        virtual bool isSpecialCategory() const;

        virtual void setItems( const QStringList& items );
        virtual void removeItem( const QString& item );
        virtual void renameItem( const QString& oldValue, const QString& newValue );
        virtual void addItem( const QString& item );
        virtual QStringList items() const;
        virtual QStringList itemsInclGroups() const;

    private:
        QString _category;
    };
}


#endif /* SQLCATEGORY_H */

