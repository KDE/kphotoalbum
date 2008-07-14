#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <kdialog.h>
class QGridLayout;
class QLabel;
class QGroupBox;
class QComboBox;
class QTreeWidgetItem;
class QTreeWidget;

namespace DB { class ImageSearchInfo; }
namespace MainWindow {

class StatisticsDialog :public KDialog
{
    Q_OBJECT

public:
    StatisticsDialog( QWidget* parent );
    OVERRIDE QSize sizeHint() const;
    void show();

private:
    QTreeWidgetItem* addRow( const QString& title, int noOfTags, int tagCount, int imageCount, QTreeWidgetItem* parent );
    void highlightTotalRow( QTreeWidgetItem* item );
    QGroupBox* createAnnotatedGroupBox();
    void populateSubTree( const DB::ImageSearchInfo& info, int imageCount, QTreeWidgetItem* top );

private slots:
    void categoryChanged(int);
    void populate();

private:
    QTreeWidget* m_treeWidget;
    QComboBox* m_category;
    QLabel* m_tagLabel;
    QComboBox* m_tag;
    QGridLayout* m_boxLayout;
};

}

#endif /* STATISTICSDIALOG_H */

