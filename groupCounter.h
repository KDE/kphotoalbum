#ifndef GROUPCOUNTER_H
#define GROUPCOUNTER_H
#include "options.h"
#include <qdict.h>

class GroupCounter
{
public:
    GroupCounter( const QString& optionGroup );
    void count(const QStringList& );
    QMap<QString,int> result();

private:
    QDict<QStringList> _memberToGroup;
    QDict<int> _groupCount;

};

#endif /* GROUPCOUNTER_H */

