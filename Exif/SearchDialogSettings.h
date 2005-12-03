#ifndef SEARCHDIALOGSETTINGS_H
#define SEARCHDIALOGSETTINGS_H
#include <qcheckbox.h>

namespace Exif{

class IntValueSetting
{
public:
    IntValueSetting() {}
    IntValueSetting( QCheckBox* cb, int value ) : cb( cb ), value( value ) {}
    QCheckBox* cb;
    int value;
};

class IntSettings :public QValueList<IntValueSetting>
{
public:
    QValueList<int> selected();
};

}


#endif /* SEARCHDIALOGSETTINGS_H */

