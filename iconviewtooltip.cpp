#include "iconviewtooltip.h"
#include <qcursor.h>
#include "thumbnail.h"
#include <qlayout.h>
#include "util.h"
#include <qtooltip.h>
#include "options.h"

/**
   This class takes care of showing tooltips for the individual items in the iconview.
   I tried implementing this with QToolTip::maybeTip() on the iconview, but it had two
   disadvantages: (1) When scrolling using the scrollbutton of the mouse,
   the tooltip was not updated. (2) Either the tooltip would not follow the
   mouse( and would therefore stand on top of the image), or it flickered.
*/
IconViewToolTip::IconViewToolTip( QIconView* view, const char* name )
    : QLabel( view, name, WStyle_Customize | WStyle_NoBorder | WType_TopLevel | WX11BypassWM | WStyle_Tool ), _view( view ), _showing( false ), _current(0)
{
    view->viewport()->installEventFilter( this );
	setAlignment( AlignAuto | AlignTop );
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth(1);
    setMargin(1);
    setPalette( QToolTip::palette() );
}

bool IconViewToolTip::eventFilter( QObject*, QEvent* e)
{
    if ( _showing ) {
        if ( QEvent::MouseButtonPress <= e->type() &&
             e->type() <= QEvent::MouseButtonDblClick || e->type() == QEvent::WindowDeactivate ) {
            _showing = false;
            hide();
        }
        else {
            showToolTips();
        }
    }

    return false;
}

void IconViewToolTip::showToolTips()
{
    _showing = true;
    QIconViewItem* item = itemAtCursor();
    if ( item ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        setText( QString::fromLatin1("<qt>") + Util::createInfoText( tn->imageInfo(), 0 ) + QString::fromLatin1("</qt") );

        _current = item;
        move( QCursor::pos() + QPoint( 10, 10 ));
        show();
        resize( sizeHint() );
        _view->setFocus();
    }
}


QIconViewItem* IconViewToolTip::itemAtCursor()
{
    QPoint pos = QCursor::pos();
    int x,y;
    pos = _view->mapFromGlobal( pos );
    _view->viewportToContents( pos.x(), pos.y(), x, y );
    QIconViewItem* item = _view->findItem( QPoint(x,y) );
    return item;
}

#include "iconviewtooltip.moc"
