#ifndef MEMBERMAP_H
#define MEMBERMAP_H
#include <qstringlist.h>
#include <qdom.h>
#include <qmap.h>
#include <qobject.h>

class MemberMap :public QObject {
    Q_OBJECT
public:
    MemberMap();
    MemberMap( const MemberMap& );
    MemberMap& operator=( const MemberMap& );

    QStringList groups( const QString& optionGroup ) const;
    void deleteGroup( const QString& optionGroup, const QString& name );
    QStringList members( const QString& optionGroup, const QString& memberGroup, bool closure ) const;
    void setMembers( const QString& optionGroup, const QString& memberGroup, const QStringList& members );
    QDomElement save( QDomDocument doc );
    bool isEmpty() const;
    void load( const QDomElement& );
    bool isGroup( const QString& optionGroup, const QString& memberGroup ) const;
    QMap<QString,QStringList> groupMap( const QString& optionGroup );
    void renameGroup( const QString& optionGroup, const QString& oldName, const QString& newName );

protected:
    void calculate();
    QStringList calculateClosure( QMap<QString,QStringList>& resultSoFar, const QString& optionGroup, const QString& group );

protected slots:
    void init();
    void deleteOption( const QString& optionGroup, const QString& name);
    void renameOption( const QString& optionGroup, const QString& oldName, const QString& newName );

private:
    // This is the primary data structure
    // { optionGroup |-> { group |-> [ member ] } } <- VDM syntax ;-)
    QMap<QString, QMap<QString,QStringList> > _members;

    // These are the data structures used to develop closures, they are only
    // needed to speed up the program *SIGNIFICANTLY* ;-)
    bool _dirty;
    QMap<QString, QMap<QString,QStringList> > _closureMembers;

};

#endif /* MEMBERMAP_H */

