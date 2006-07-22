#ifndef DB_CATEGORYITEMS_H
#define DB_CATEGORYITEMS_H

/**
USA
  Nevada
    Las Vegas
  California
    San Fransisco
    Los Angeless
      Long Beach
Esbjerg

*/

namespace DB
{
class CategoryItem
{
public:
    CategoryItem( const QString& name ) : _name( name ) {}
    void print( int offset )
    {
        QString spaces;
        spaces.fill( ' ', offset );
        qDebug( "%s%s", spaces.latin1(), _name.latin1() );
        for( QValueList< CategoryItem* >::Iterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
            (*it)->print( offset + 2 );
        }
    }

    QString _name;
    QValueList< CategoryItem* > _subcategories;
};

}


#endif /* DB_CATEGORYITEMS_H */

