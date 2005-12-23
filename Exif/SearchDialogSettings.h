#ifndef SEARCHDIALOGSETTINGS_H
#define SEARCHDIALOGSETTINGS_H
#include <qcheckbox.h>

namespace Exif{

template <class T>
class Setting
{
public:
    Setting() {}
    Setting( QCheckBox* cb, T value ) : cb( cb ), value( value ) {}
    QCheckBox* cb;
    T value;
};

template <class T>
class Settings :public QValueList< Setting<T> >
{
public:
    QValueList<T> selected()
    {
        QValueList<T> result;
        for( typename QValueList< Setting<T> >::Iterator it = this->begin(); it != this->end(); ++it ) {
            if ( (*it).cb->isChecked() )
                result.append( (*it).value );
        }
        return result;
    }
};

}


#endif /* SEARCHDIALOGSETTINGS_H */

