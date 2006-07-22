#ifndef ANNOTATIONDIALOG_KEYCLICKLISTENER_H
#define ANNOTATIONDIALOG_KEYCLICKLISTENER_H

#include <qobject.h>

namespace AnnotationDialog
{
class KeyClickListener :public QObject
{
    Q_OBJECT

public:
    KeyClickListener( Qt::Key, QObject* parent );

signals:
    void keyClicked();

protected:
    virtual bool eventFilter( QObject* watched, QEvent* e );

private:
    bool _pendingClick;
    Qt::Key _key;

};

}

#endif /* ANNOTATIONDIALOG_KEYCLICKLISTENER_H */

