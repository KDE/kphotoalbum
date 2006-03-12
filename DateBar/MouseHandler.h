#ifndef DATEBARMOUSEHANDLER_H
#define DATEBARMOUSEHANDLER_H
#include <qevent.h>
#include <qobject.h>
#include <qdatetime.h>
#include <kdemacros.h>
#include <kdeversion.h>
#include <imagedate.h>

#if ! KDE_IS_VERSION(3,3,90)
#undef KDE_EXPORT
#define KDE_EXPORT
#endif

namespace DateBar {
class DateBar;

    class KDE_EXPORT MouseHandler : public QObject
    {
        Q_OBJECT
    public:
        MouseHandler( DateBar* dateBar );
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



    class KDE_EXPORT FocusItemDragHandler : public MouseHandler
    {
    public:
        FocusItemDragHandler( DateBar* dateBar );
        void mousePressEvent( int x );
        void mouseMoveEvent( int x );
    };



    class KDE_EXPORT BarDragHandler : public MouseHandler
    {
    public:
        BarDragHandler( DateBar* );
        void mousePressEvent( int x );
        void mouseMoveEvent(  int x );
    private:
        int _movementOffset;
    };



    class KDE_EXPORT SelectionHandler : public MouseHandler
    {
    public:
        SelectionHandler( DateBar* );
        void mousePressEvent( int x );
        void mouseMoveEvent( int x );
        virtual void mouseReleaseEvent();
        QDateTime min() const;
        QDateTime max() const;
        ImageDate dateRange() const;
        void clearSelection();
        bool hasSelection() const;
    private:
        QDateTime _start;
        QDateTime _end;
    };
}

#endif /* DATEBARMOUSEHANDLER_H */

