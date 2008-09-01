#ifndef INFOBOXRESIZER_H
#define INFOBOXRESIZER_H

#include <QPoint>
namespace Viewer {class InfoBox;}

namespace Viewer {

class InfoBoxResizer
{
public:
    InfoBoxResizer( Viewer::InfoBox* infoBox );
    void setup( bool left, bool right, bool top, bool bottom );
    void setPos( QPoint pos );
    void deactivate();
    bool isActive() const;

private:
    InfoBox* _infoBox;
    bool _left;
    bool _right;
    bool _top;
    bool _bottom;
    bool _active;
};

}

#endif /* INFOBOXRESIZER_H */

