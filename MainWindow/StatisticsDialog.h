#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <kdialog.h>
class QTreeWidgetItem;
class QTreeWidget;

namespace MainWindow {

class StatisticsDialog :public KDialog
{
public:
    StatisticsDialog( QWidget* parent );
    OVERRIDE QSize sizeHint() const;

protected:
    OVERRIDE void showEvent ( QShowEvent * event );
    QTreeWidgetItem* addRow( const QString& title, int tagCount, int total );
    void highlightTotalRow( QTreeWidgetItem* item );
private:
    QTreeWidget* m_treeWidget;
};

}

#endif /* STATISTICSDIALOG_H */

