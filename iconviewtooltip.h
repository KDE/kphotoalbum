#ifndef ICONVIEWTOOLTIP_H
#define ICONVIEWTOOLTIP_H
#include <qiconview.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qdialog.h>

class IconViewToolTip :public QLabel {
    Q_OBJECT

public:
    IconViewToolTip( QIconView* view, const char* name = 0 );

protected:
    virtual bool eventFilter ( QObject*, QEvent* e );
    QIconViewItem* itemAtCursor();

protected slots:
    void showToolTip();

private:
    QTimer* _timer;
    QIconView* _view;
    bool _showing;
    QIconViewItem* _current;
};


#endif /* ICONVIEWTOOLTIP_H */

