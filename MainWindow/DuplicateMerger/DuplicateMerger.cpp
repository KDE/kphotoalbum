/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DuplicateMerger.h"
#include "DB/ImageDB.h"
#include "DB/FileName.h"
#include "DB/FileNameList.h"
#include "DB/ImageInfo.h"
#include "DB/MD5.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include "DuplicateMatch.h"
#include <KLocale>
#include <QLabel>
#include "Utilities/ShowBusyCursor.h"
#include "MergeToolTip.h"
#include <QRadioButton>
#include "Utilities/DeleteFiles.h"

namespace MainWindow {

DuplicateMerger::DuplicateMerger(QWidget *parent) :
    KDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(800,600);

    QWidget* top = new QWidget(this);
    QVBoxLayout* topLayout = new QVBoxLayout(top);
    setMainWidget(top);

    QString txt = i18n("<p>Below is a list of all images that are duplicate in your database.<br/>"
                       "Select which you want merged, and which of the duplicates should be kept.<br/>"
                       "The tag and description from the deleted images will be transferred to the kept image</p>");
    QLabel* label = new QLabel(txt);
    QFont fnt = font();
    fnt.setPixelSize(18);
    label->setFont(fnt);
    topLayout->addWidget(label);

    _trash = new QRadioButton(i18n("Move to &trash"));
    _deleteFromDisk = new QRadioButton(i18n("&Delete from disk"));
    QRadioButton* blockFromDB = new QRadioButton(i18n("&Block from database"));
    _trash->setChecked(true);

    topLayout->addSpacing(10);
    topLayout->addWidget(_trash);
    topLayout->addWidget(_deleteFromDisk);
    topLayout->addWidget(blockFromDB);
    topLayout->addSpacing(10);

    QScrollArea* scrollArea = new QScrollArea;
    topLayout->addWidget(scrollArea);
    scrollArea->setWidgetResizable(true);

    m_container = new QWidget(scrollArea);
    m_scrollLayout = new QVBoxLayout(m_container);
    scrollArea->setWidget(m_container);

    m_selectionCount = new QLabel;
    topLayout->addWidget(m_selectionCount);

    setButtons(Ok|Cancel|User1|User2);
    setButtonText(User1, i18n("Select &All"));
    setButtonText(User2, i18n("Select &None"));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(selectAll()));
    connect(this, SIGNAL(user2Clicked()), this, SLOT(selectNone()));
    connect(this, SIGNAL(okClicked()), this, SLOT(go()));
    findDuplicates();
}

MainWindow::DuplicateMerger::~DuplicateMerger()
{
    MergeToolTip::destroy();
}

void DuplicateMerger::selectAll()
{
    selectAll(true);
}

void DuplicateMerger::selectNone()
{
    selectAll(false);
}

void DuplicateMerger::go()
{
    Utilities::DeleteMethod method = Utilities::BlockFromDatabase;
    if (_trash->isChecked())
        method = Utilities::MoveToTrash;
    else if (_deleteFromDisk->isChecked())
        method = Utilities::DeleteFromDisk;
    Q_FOREACH( DuplicateMatch* selector, m_selectors) {
        selector->execute(method);
    }
}

void DuplicateMerger::updateSelectionCount()
{
    int total = 0;
    int selected = 0;
    Q_FOREACH( DuplicateMatch* selector, m_selectors) {
        ++total;
        if (selector->selected())
            ++selected;
    }
    m_selectionCount->setText(i18n("%1 of %2 selected", selected, total));

}

void DuplicateMerger::findDuplicates()
{
    Utilities::ShowBusyCursor dummy;
    Q_FOREACH( const DB::FileName& fileName, DB::ImageDB::instance()->images() ) {
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const DB::MD5 md5 = info->MD5Sum();
        m_matches[md5].append(fileName);
    }

    bool anyFound = false;
    for (QMap<DB::MD5, DB::FileNameList>::const_iterator it = m_matches.constBegin(); it != m_matches.constEnd(); ++it) {
        if ( it.value().count() > 1 ) {
            addRow(it.key());
            anyFound = true;
        }
    }

    if ( !anyFound )
        tellThatNoDuplicatesWereFound();
    updateSelectionCount();
}

void DuplicateMerger::addRow(const DB::MD5 &md5)
{
    DuplicateMatch* match = new DuplicateMatch( m_matches[md5]);
    connect( match, SIGNAL(selectionChanged()), this, SLOT(updateSelectionCount()));
    m_scrollLayout->addWidget(match);
    m_selectors.append(match);
}

void DuplicateMerger::selectAll(bool b)
{
    Q_FOREACH( DuplicateMatch* selector, m_selectors) {
        selector->setSelected(b);
    }
}

void DuplicateMerger::tellThatNoDuplicatesWereFound()
{
    QLabel* label = new QLabel(i18n("No duplicates found"));
    QFont fnt = font();
    fnt.setPixelSize(30);
    label->setFont(fnt);
    m_scrollLayout->addWidget(label);

    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(Cancel, false);
}

} // namespace MainWindow

#include "DuplicateMerger.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
