#include "KeyClickListener.h"
#include <qevent.h>

/**
 * \class AnnotationDialog::KeyClickListener
 * \brief Instances of this class listens to key clicks of a given key, and notifies using the \ref keyClicked signal.
 *
 * This class is used to listen to key clicks (key press imidiately followed by a key release) on any key.
 * This may especially be used for key click of keys that normally are passive, like control and alt.
 */
AnnotationDialog::KeyClickListener::KeyClickListener( Qt::Key key , QObject* parent )
    :QObject( parent ), _pendingClick( false ), _key( key )
{
    parent->installEventFilter( this );
}

bool AnnotationDialog::KeyClickListener::eventFilter( QObject*, QEvent* event )
{
    if ( event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease )
        return false;

    QKeyEvent* kev = static_cast<QKeyEvent*>( event );

    if ( kev->key() != _key )
        _pendingClick = false;

    else {
        if ( event->type() == QEvent::KeyPress && kev->state() == Qt::NoButton )
            _pendingClick = true;

        else if ( event->type() == QEvent::KeyRelease && _pendingClick ) {
            _pendingClick = false;
            emit keyClicked();
        }
    }

    return false;
}

