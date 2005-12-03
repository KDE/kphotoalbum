#ifndef EXIFSEARCHINFO_H
#define EXIFSEARCHINFO_H

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qpair.h>

class ExifSearchInfo  {

public:
    void addSearchKey( const QString& key, const QValueList<int> values );
    QStringList matches();
    QString buildQuery();

private:
    QValueList< QPair<QString, QValueList<int> > > _intKeys;

};


#endif /* EXIFSEARCHINFO_H */

