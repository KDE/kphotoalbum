#ifndef INFOBOX_H
#define INFOBOX_H
#include <qtextbrowser.h>
class Viewer;

class InfoBox :public QTextBrowser {
    Q_OBJECT

public:
    InfoBox( Viewer* parent, const char* name = 0 );
    void setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap );
    virtual void setSource( const QString& which );

protected:
    virtual void contentsMouseMoveEvent( QMouseEvent* );

private:
    QMap<int, QPair<QString,QString> > _linkMap;
    Viewer* _viewer;
};


#endif /* INFOBOX_H */

