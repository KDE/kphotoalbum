#ifndef MOUSETRACKINGINTERACTION_H
#define MOUSETRACKINGINTERACTION_H

#include "MouseInteraction.h"

namespace ThumbnailView
{
class ThumbnailWidget;

class MouseTrackingInteraction : public MouseInteraction {

public:
    MouseTrackingInteraction( ThumbnailWidget* );
    virtual void mouseMoveEvent( QMouseEvent* );

private:
    ThumbnailWidget* _view;
};

}

#endif /* MOUSETRACKINGINTERACTION_H */

