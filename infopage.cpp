#include "infopage.h"
#include <qlabel.h>
#include <kstandarddirs.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <klocale.h>
InfoPage::InfoPage( QWidget* parent, const char* name )
    :QWidget( parent, name )
{
    QHBoxLayout* layout = new QHBoxLayout( this, 10, 6 );
    layout->addStretch(1);

    QLabel* label = new QLabel( this );
    layout->addWidget( label );
    label->setPixmap( QPixmap( locate("data", QString::fromLatin1("kimdaba/pics/splash.png") ) ) );

    label = new QLabel( i18n("<qt><center><font size=\"+3\">Welcome to KimDaba</font></center><p>"
                             "To see your images, please either start a search using <b>Edit|Find</b> "
                             "or chose <b>Images|Display All Thumbnails</b>.</qt>" ), this );
    layout->addWidget( label );
    layout->addStretch(1);
}
