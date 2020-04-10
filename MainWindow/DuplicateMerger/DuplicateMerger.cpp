/* Copyright 2012-2020 The KPhotoAlbum Development Team

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

#include "DuplicateMatch.h"
#include "MergeToolTip.h"

#include <DB/FileName.h>
#include <DB/FileNameList.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/MD5.h>
#include <Utilities/DeleteFiles.h>
#include <Utilities/ShowBusyCursor.h>

#include <KLocalizedString>
#include <QDebug>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace MainWindow
{

DuplicateMerger::DuplicateMerger(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(800, 600);

    QWidget *top = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(top);
    setLayout(topLayout);
    topLayout->addWidget(top);

    QString txt = i18n("<p>Below is a list of all images that are duplicate in your database.<br/>"
                       "Select which you want merged, and which of the duplicates should be kept.<br/>"
                       "The tag and description from the deleted images will be transferred to the kept image</p>");
    QLabel *label = new QLabel(txt);
    QFont fnt = font();
    fnt.setPixelSize(18);
    label->setFont(fnt);
    topLayout->addWidget(label);

    m_trash = new QRadioButton(i18n("Move to &trash"));
    m_deleteFromDisk = new QRadioButton(i18n("&Delete from disk"));
    QRadioButton *blockFromDB = new QRadioButton(i18n("&Block from database"));
    m_trash->setChecked(true);

    topLayout->addSpacing(10);
    topLayout->addWidget(m_trash);
    topLayout->addWidget(m_deleteFromDisk);
    topLayout->addWidget(blockFromDB);
    topLayout->addSpacing(10);

    QScrollArea *scrollArea = new QScrollArea;
    topLayout->addWidget(scrollArea);
    scrollArea->setWidgetResizable(true);

    m_container = new QWidget(scrollArea);
    m_scrollLayout = new QVBoxLayout(m_container);
    scrollArea->setWidget(m_container);

    m_selectionCount = new QLabel;
    topLayout->addWidget(m_selectionCount);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();

    m_selectAllButton = buttonBox->addButton(i18n("Select &All"), QDialogButtonBox::YesRole);
    m_selectNoneButton = buttonBox->addButton(i18n("Select &None"), QDialogButtonBox::NoRole);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(m_selectAllButton, &QPushButton::clicked, this, QOverload<bool>::of(&DuplicateMerger::selectAll));
    connect(m_selectNoneButton, &QPushButton::clicked, this, &DuplicateMerger::selectNone);
    connect(m_okButton, &QPushButton::clicked, this, &DuplicateMerger::go);
    connect(m_cancelButton, &QPushButton::clicked, this, &DuplicateMerger::reject);

    topLayout->addWidget(buttonBox);

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

    if (m_trash->isChecked()) {
        method = Utilities::MoveToTrash;
    } else if (m_deleteFromDisk->isChecked()) {
        method = Utilities::DeleteFromDisk;
    }

    for (DuplicateMatch *selector : qAsConst(m_selectors)) {
        selector->execute(method);
    }

    accept();
}

void DuplicateMerger::updateSelectionCount()
{
    int total = 0;
    int selected = 0;

    for (const DuplicateMatch *selector : qAsConst(m_selectors)) {
        ++total;
        if (selector->selected())
            ++selected;
    }

    m_selectionCount->setText(i18n("%1 of %2 selected", selected, total));
    m_okButton->setEnabled(selected > 0);
}

void DuplicateMerger::findDuplicates()
{
    Utilities::ShowBusyCursor dummy;

    const auto images = DB::ImageDB::instance()->images();
    for (const DB::FileName &fileName : images) {
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const DB::MD5 md5 = info->MD5Sum();
        m_matches[md5].append(fileName);
    }

    bool anyFound = false;
    for (QMap<DB::MD5, DB::FileNameList>::const_iterator it = m_matches.constBegin();
         it != m_matches.constEnd(); ++it) {
        if (it.value().count() > 1) {
            addRow(it.key());
            anyFound = true;
        }
    }

    if (!anyFound) {
        tellThatNoDuplicatesWereFound();
    }

    updateSelectionCount();
}

void DuplicateMerger::addRow(const DB::MD5 &md5)
{
    DuplicateMatch *match = new DuplicateMatch(m_matches[md5]);
    connect(match, &DuplicateMatch::selectionChanged, this, &DuplicateMerger::updateSelectionCount);
    m_scrollLayout->addWidget(match);
    m_selectors.append(match);
}

void DuplicateMerger::selectAll(bool b)
{
    for (DuplicateMatch *selector : qAsConst(m_selectors)) {
        selector->setSelected(b);
    }
}

void DuplicateMerger::tellThatNoDuplicatesWereFound()
{
    QLabel *label = new QLabel(i18n("No duplicates found"));
    QFont fnt = font();
    fnt.setPixelSize(30);
    label->setFont(fnt);
    m_scrollLayout->addWidget(label);

    m_selectAllButton->setEnabled(false);
    m_selectNoneButton->setEnabled(false);
    m_okButton->setEnabled(false);
}

} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:
