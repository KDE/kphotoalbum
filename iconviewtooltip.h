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
    void showToolTips();

protected:
    virtual bool eventFilter ( QObject*, QEvent* e );
    QIconViewItem* itemAtCursor();

private:
    QIconView* _view;
    bool _showing;
    QIconViewItem* _current;
};


#endif /* ICONVIEWTOOLTIP_H */

