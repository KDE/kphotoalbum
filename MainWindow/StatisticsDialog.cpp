#include "StatisticsDialog.h"
#include <QLabel>
#include <QFormLayout>
#include <QHeaderView>
#include "DB/ImageDB.h"
#include <klocale.h>
#include <QTreeWidget>
#include <QVBoxLayout>
#include "DB/Category.h"
using namespace MainWindow;

StatisticsDialog::StatisticsDialog( QWidget* parent )
    : KDialog( parent )
{
    QWidget* top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );
    setMainWidget(top);

    QString txt = i18n("<h1>Description</h1>"
                       "<table>"
                       "<tr><td># of Items</td><td>This is the number of different items in the category</td></tr>"
                       "<tr><td>Tags Total</td><td>This is a count of how many tags was made,<br/>i.e. a simple counting though all the images</tr></tr>"
                       "<tr><td>Tags Per Picture</td><td>This tells you how many tags are on each picture on average</td></tr>"
                       "</table>");

    QLabel* label = new QLabel(txt);
    layout->addWidget( label );

    label = new QLabel("<h1>Statistics</h1>");
    layout->addWidget(label);

    m_treeWidget = new QTreeWidget;
    layout->addWidget( m_treeWidget );

    QStringList labels;
    labels << i18n("Category") << i18n("# of Items") << i18n("Tags Totals") << i18n("Tags Per Picture") << QString();
    m_treeWidget->setHeaderLabels( labels );
}

void StatisticsDialog::showEvent( QShowEvent * event )
{
    m_treeWidget->clear();
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();

    int tagsTotal = 0;
    int grantTotal = 0;
    Q_FOREACH( const DB::CategoryPtr& category, categories ) {
        if ( category->name() == "Media Type" || category->name() == "Folder")
            continue;

        const QMap<QString,uint> tags = DB::ImageDB::instance()->classify( DB::ImageSearchInfo(), category->name(), DB::anyMediaType );
        int total = 0;
        for( QMap<QString,uint>::ConstIterator tagIt = tags.constBegin(); tagIt != tags.constEnd(); ++tagIt ) {
            if ( tagIt.key() != DB::ImageDB::NONE() )
                total += tagIt.value();
        }

        addRow( category->text(), tags.count()-1, total );
        tagsTotal += tags.count() -1;
        grantTotal += total;
    }

    QTreeWidgetItem* totalRow = addRow( i18n("Total"), tagsTotal, grantTotal );
    highlightTotalRow( totalRow );

    m_treeWidget->header()->resizeSections( QHeaderView::ResizeToContents );
}

QSize MainWindow::StatisticsDialog::sizeHint() const
{
    return QSize( 800, 600 );
}

QTreeWidgetItem* MainWindow::StatisticsDialog::addRow( const QString& title, int tagCount, int total )
{
    QStringList list;
    list << title
         << QString::number(tagCount)
         << QString::number( total )
         << QString::number( (double) total / DB::ImageDB::instance()->totalCount(), 'F', 2);
    QTreeWidgetItem* item = new QTreeWidgetItem( m_treeWidget, list );
    for (int col =1;col <4; ++col )
        item->setTextAlignment( col, Qt::AlignRight );
    return item;
}

void MainWindow::StatisticsDialog::highlightTotalRow( QTreeWidgetItem* item )
{
    for ( int col=0; col<5; ++col ) {
        QFont font = item->data( col, Qt::FontRole ).value<QFont>();
        font.setWeight( QFont::Bold );
        item->setData( col, Qt::FontRole, font );
    }
}
