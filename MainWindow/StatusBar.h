#ifndef STATUSBAR_H
#define STATUSBAR_H
#include "BreadcrumbViewer.h"
#include <KStatusBar>
class QLabel;

namespace MainWindow {
class ImageCounter;
class DirtyIndicator;

class StatusBar :public KStatusBar
{
public:
    StatusBar();
    DirtyIndicator* _dirtyIndicator;
    ImageCounter* _partial;
    BreadcrumbViewer* _pathIndicator;

    void setLocked( bool locked );

private:
    QLabel* _lockedIndicator;
    void setupFixedFonts();
    void setupGUI();
};

}


#endif /* STATUSBAR_H */

