#include "SearchDialogSettings.h"

using namespace Exif;

QValueList<int> IntSettings::selected()
{
    QValueList<int> result;
    for( QValueList<IntValueSetting>::Iterator it = begin(); it != end(); ++it ) {
        if ( (*it).cb->isChecked() )
            result.append( (*it).value );
    }
    return result;
}


