#ifndef CATEGORYIMAGECOLLECTION_H
#define CATEGORYIMAGECOLLECTION_H

#include "myimagecollection.h"
#include "imagesearchinfo.h"

class CategoryImageCollection :public MyImageCollection {

public:
    CategoryImageCollection( const ImageSearchInfo& context, const QString& optionGroup, const QString& value );
    virtual QString name();
    virtual KURL::List images();
private:
    ImageSearchInfo _context;
    const QString _optionGroup;
    const QString _value;
};


#endif /* CATEGORYIMAGECOLLECTION_H */

