/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "StatisticsDialog.h"

#include "DB/CategoryCollection.h"
#include "DB/Category.h"
#include "DB/ImageDB.h"
#include "DB/ImageSearchInfo.h"
#include "Utilities/ShowBusyCursor.h"

#include <KComboBox>
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace MainWindow;

StatisticsDialog::StatisticsDialog( QWidget* parent )
    : QDialog( parent )
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QString txt = i18n("<h1>Description</h1>"
                       "<table>"
                       "<tr><td># of Items</td><td>This is the number of different items in the category</td></tr>"
                       "<tr><td>Tags Total</td><td>This is a count of how many tags was made,<br/>i.e. a simple counting though all the images</td></tr>"
                       "<tr><td>Tags Per Picture</td><td>This tells you how many tags are on each picture on average</td></tr>"
                       "</table><br/><br/>"
                       "Do not get too attached to this dialog, it has the problem that it counts categories AND subcategories,<br/>"
                       "so if an image has been taken in Las Vegas, Nevada, USA, then 3 tags are counted for that image,<br/>"
                       "while it should only be one.<br/>"
                       "I am not really sure if it is worth fixing that bug (as it is pretty hard to fix),<br/>"
                       "so maybe the dialog will simply go away again");

    QLabel* label = new QLabel(txt);
    layout->addWidget( label );

    layout->addWidget( createAnnotatedGroupBox() );

    label = new QLabel(i18n("<h1>Statistics</h1>"));
    layout->addWidget(label);

    m_treeWidget = new QTreeWidget;
    layout->addWidget( m_treeWidget );

    QStringList labels;
    labels << i18n("Category") << i18n("# of Items") << i18n("Tags Totals") << i18n("Tags Per Picture") << QString();
    m_treeWidget->setHeaderLabels( labels );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

void StatisticsDialog::show()
{
    populate();
    QDialog::show();
}

QSize MainWindow::StatisticsDialog::sizeHint() const
{
    return QSize( 800, 800 );
}

QTreeWidgetItem* MainWindow::StatisticsDialog::addRow( const QString& title, int noOfTags, int tagCount, int imageCount, QTreeWidgetItem* parent )
{
    QStringList list;
    list << title
         << QString::number(noOfTags)
         << QString::number( tagCount )
         << QString::number( (double) tagCount / imageCount, 'F', 2);
    QTreeWidgetItem* item = new QTreeWidgetItem( parent, list );
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

QGroupBox* MainWindow::StatisticsDialog::createAnnotatedGroupBox()
{
    QGroupBox* box = new QGroupBox( i18n("Tag indication completed annotation") );

    m_boxLayout = new QGridLayout(box);
    m_boxLayout->setColumnStretch(2,1);
    int row = -1;

    QLabel* label = new QLabel(i18n("If you use a specific tag to indicate that an image has been tagged, then specify it here.") );
    label->setWordWrap( true );
    m_boxLayout->addWidget( label, ++row, 0, 1, 3 );

    label = new QLabel( i18n("Category:") );
    m_boxLayout->addWidget( label, ++row, 0 );

    m_category = new KComboBox;
    m_boxLayout->addWidget( m_category, row, 1 );

    m_tagLabel = new QLabel(i18n("Tag:") );
    m_boxLayout->addWidget( m_tagLabel, ++row, 0 );

    m_tag = new KComboBox;
    m_tag->setSizeAdjustPolicy(KComboBox::AdjustToContents);
    m_boxLayout->addWidget( m_tag, row, 1 );

    m_category->addItem( i18nc("@item:inlistbox meaning 'no category'","None") );

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    Q_FOREACH( const DB::CategoryPtr& category, categories ) {
        if (category->type() == DB::Category::MediaTypeCategory
                || category->type() == DB::Category::FolderCategory)
        {
            continue;
        }
        m_category->addItem(category->name(), category->name());
    }

    connect(m_category, static_cast<void (KComboBox::*)(int)>(&KComboBox::activated), this, &StatisticsDialog::categoryChanged);
    connect(m_tag, static_cast<void (KComboBox::*)(int)>(&KComboBox::activated), this, &StatisticsDialog::populate);
    m_tagLabel->setEnabled(false);
    m_tag->setEnabled(false);

    return box;
}

void MainWindow::StatisticsDialog::categoryChanged(int index)
{
    const bool enabled = (index != 0 );
    m_tagLabel->setEnabled( enabled );
    m_tag->setEnabled( enabled );

    m_tag->clear();

    if ( enabled ) {
        const QString name =  m_category->itemData(index).value<QString>();
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( name );
        m_tag->addItems( category->items() );
    }
}

void MainWindow::StatisticsDialog::populate()
{
    Utilities::ShowBusyCursor dummy;
    m_treeWidget->clear();

    const int imageCount = DB::ImageDB::instance()->totalCount();
    QTreeWidgetItem* top = new QTreeWidgetItem( m_treeWidget, QStringList() << i18nc("As in 'all images'","All") << QString::number(imageCount) );
    top->setTextAlignment( 1, Qt::AlignRight );
    populateSubTree( DB::ImageSearchInfo(), imageCount, top );

    if ( m_category->currentIndex() != 0 ) {
        const QString category = m_category->itemData(m_category->currentIndex()).value<QString>();
        const QString tag = m_tag->currentText();
        DB::ImageSearchInfo info;
        info.setCategoryMatchText( category, tag );
        const int imageCount = DB::ImageDB::instance()->count(info ).total();
        QTreeWidgetItem* item = new QTreeWidgetItem( m_treeWidget,
                                                     QStringList() << QString::fromLatin1("%1: %2").arg(category).arg(tag)
                                                     << QString::number(imageCount));
        item->setTextAlignment( 1, Qt::AlignRight );
        populateSubTree( info, imageCount, item );
    }
    m_treeWidget->header()->resizeSections( QHeaderView::ResizeToContents );
}

void MainWindow::StatisticsDialog::populateSubTree( const DB::ImageSearchInfo& info, int imageCount, QTreeWidgetItem* top )
{
    top->setExpanded(true);

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();

    int tagsTotal = 0;
    int grantTotal = 0;
    Q_FOREACH( const DB::CategoryPtr& category, categories ) {
        if (category->type() == DB::Category::MediaTypeCategory
                || category->type() == DB::Category::FolderCategory)
        {
            continue;
        }

        const QMap<QString, DB::CategoryClassification> tags = DB::ImageDB::instance()->classify( info, category->name(), DB::anyMediaType );
        int total = 0;
        for( auto tagIt = tags.constBegin(); tagIt != tags.constEnd(); ++tagIt ) {
            // Don't count the NONE tag, and the OK tag
            if ( tagIt.key() != DB::ImageDB::NONE() && ( category->name() != m_category->currentText() || tagIt.key() != m_tag->currentText() ) )
                total += tagIt.value().count;
        }


        addRow(category->name(), tags.count()-1, total, imageCount, top);
        tagsTotal += tags.count() -1;
        grantTotal += total;
    }

    QTreeWidgetItem* totalRow = addRow( i18n("Total"), tagsTotal, grantTotal, imageCount, top );
    highlightTotalRow( totalRow );

}
// vi:expandtab:tabstop=4 shiftwidth=4:
