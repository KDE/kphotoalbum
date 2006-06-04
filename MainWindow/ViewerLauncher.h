#ifndef MAINVIEW_LAUNCHER_H
#define MAINVIEW_LAUNCHER_H
#include <qstringlist.h>

namespace MainWindow
{

class ViewerLauncher
{
public:
    static void launch( const QStringList& files, bool reuse, bool slideShow, bool random );

protected:
    static void launchImages(QStringList images, bool reuse, bool slideShow, bool random );
};

}

#endif /* MAINVIEW_LAUNCHER_H */

