#ifndef ANNOTATIONDIALOG_SHOWSELECTIONONLYMANAGER_H
#define ANNOTATIONDIALOG_SHOWSELECTIONONLYMANAGER_H

#include <qobject.h>

namespace AnnotationDialog
{
class ShowSelectionOnlyManager :public QObject
{
    Q_OBJECT

public:
    static ShowSelectionOnlyManager& instance();

public slots:
    void toggle();
    void unlimitFromSelection();

signals:
    void limitToSelected();
    void broaden();

private:
    ShowSelectionOnlyManager();
    bool _limit;

};

}

#endif /* ANNOTATIONDIALOG_SHOWSELECTIONONLYMANAGER_H */

