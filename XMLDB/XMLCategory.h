#ifndef XMLCATEGORY_H
#define XMLCATEGORY_H
#include "DB/Category.h"

namespace XMLDB {
    class XMLCategory :public DB::Category
    {
        Q_OBJECT

    public:
        XMLCategory( const QString& name, const QString& icon, ViewSize size, ViewType type, bool show = true );

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
        int idForName( const QString& name ) const;
        void initIdMap();
        void setIdMapping( const QString& name, int id );
        QString nameForId( int id ) const;

    private:
        QString _name;
        QString _icon;
        bool _show;
        ViewSize _size;
        ViewType _type;

        bool _isSpecial;
        QStringList _items;
        QMap<QString,int> _idMap;
        QMap<int,QString> _nameMap;
    };
}

#endif /* XMLCATEGORY_H */

