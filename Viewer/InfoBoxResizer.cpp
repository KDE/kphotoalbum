#include "InfoBoxResizer.h"
#include <QDebug>
#include "InfoBox.h"

Viewer::InfoBoxResizer::InfoBoxResizer( Viewer::InfoBox* infoBox )
    :_infoBox( infoBox )
{
}

void Viewer::InfoBoxResizer::setPos(  QPoint pos )
{
    QRect rect = _infoBox->geometry();
    pos = _infoBox->mapToParent( pos );

    if ( _left )
        rect.setLeft( pos.x() );
    if (_right)
        rect.setRight(pos.x());
    if ( _top )
        rect.setTop( pos.y() );
    if ( _bottom )
        rect.setBottom( pos.y() );

    if ( rect.width() > 100 && rect.height() > 50 )
        _infoBox->setGeometry( rect );
}

void Viewer::InfoBoxResizer::setup( bool left, bool right, bool top, bool bottom )
{
    _left = left;
    _right = right;
    _top = top;
    _bottom = bottom;
    _active = true;
}

void Viewer::InfoBoxResizer::deactivate()
{
    _active = false;
}

bool Viewer::InfoBoxResizer::isActive() const
{
    return _active;
}
