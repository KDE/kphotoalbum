#ifndef MOUSETRACKINGINTERACTION_H
#define MOUSETRACKINGINTERACTION_H

#include "MouseInteraction.h"

namespace ThumbnailView
{
class ThumbnailView;

class MouseTrackingInteraction : public MouseInteraction {

public:
    MouseTrackingInteraction( ThumbnailView* );
    virtual void mouseMoveEvent( QMouseEvent* );

private:
    ThumbnailView* _view;
};

}

#endif /* MOUSETRACKINGINTERACTION_H */

