#ifndef DATEBARMOUSEHANDLER_H
#define DATEBARMOUSEHANDLER_H
#include <qevent.h>
#include <qobject.h>
#include <qdatetime.h>
#include <kdemacros.h>
#include "imagedaterange.h"
#include <kdeversion.h>
class DateBar;

#if ! KDE_IS_VERSION(3,3,90)
#undef KDE_EXPORT
#define KDE_EXPORT
#endif

namespace DateBarMouseHandler {

    class KDE_EXPORT Handler : public QObject
    {
        Q_OBJECT
    public:
        Handler( DateBar* dateBar );
        virtual void mousePressEvent( int x ) = 0;
        virtual void mouseMoveEvent( int x ) = 0;
        virtual void mouseReleaseEvent() {};
        void startAutoScroll();
        void endAutoScroll();

    protected slots:
        void autoScroll();

    protected:
        DateBar* _dateBar;

    private:
        QTimer* _autoScrollTimer;
    };



    class KDE_EXPORT FocusItem : public Handler
    {
    public:
        FocusItem( DateBar* dateBar );
        void mousePressEvent( int x );
        void mouseMoveEvent( int x );
    };



    class KDE_EXPORT DateArea : public Handler
    {
    public:
        DateArea( DateBar* );
        void mousePressEvent( int x );
        void mouseMoveEvent(  int x );
    private:
        int _movementOffset;
    };



    class KDE_EXPORT Selection : public Handler
    {
    public:
        Selection( DateBar* );
        void mousePressEvent( int x );
        void mouseMoveEvent( int x );
        virtual void mouseReleaseEvent();
        QDateTime min() const;
        QDateTime max() const;
        ImageDateRange dateRange() const;
        void clearSelection();
        bool hasSelection() const;
    private:
        QDateTime _start;
        QDateTime _end;
    };
}

#endif /* DATEBARMOUSEHANDLER_H */

