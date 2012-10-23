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
#include "ui_DuplicateMerger.h"
#include "DB/ImageDB.h"
#include "DB/FileName.h"
#include "DB/FileNameList.h"
#include "DB/ImageInfo.h"
#include "DB/MD5.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include "DuplicateMatch.h"
namespace MainWindow {

DuplicateMerger::DuplicateMerger(QWidget *parent) :
    KDialog(parent),
    ui(new Ui::DuplicateMerger)
{
    resize(800,600);

    QWidget*top = new QWidget;
    ui->setupUi(top);
    setMainWidget(top);

    m_container = new QWidget;
    m_topLayout = new QVBoxLayout(m_container);
    ui->scrollArea->setWidget(m_container);

    setButtons(Ok|Cancel|User1|User2);
    setButtonText(User1, i18n("Select &All"));
    setButtonText(User2, i18n("Select &None"));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(selectAll()));
    connect(this, SIGNAL(user2Clicked()), this, SLOT(selectNone()));
    connect(this, SIGNAL(okClicked()), this, SLOT(go()));
    findDuplicates();
}

DuplicateMerger::~DuplicateMerger()
{
    delete ui;
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
    Q_FOREACH( DuplicateMatch* selector, m_selectors) {
        selector->execute();
    }
}

void DuplicateMerger::findDuplicates()
{
    Q_FOREACH( const DB::FileName& fileName, DB::ImageDB::instance()->images() ) {
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const DB::MD5 md5 = info->MD5Sum();
        m_matches[md5].append(fileName);
    }

    Q_FOREACH( const DB::MD5& md5, m_matches.keys() ) {
        if ( m_matches[md5].count() > 1 ) {
            addRow(md5);
        }
    }
}

void DuplicateMerger::addRow(const DB::MD5 &md5)
{
    DuplicateMatch* match = new DuplicateMatch( m_matches[md5]);
    m_topLayout->addWidget(match);
    m_selectors.append(match);
}

void DuplicateMerger::selectAll(bool b)
{
    Q_FOREACH( DuplicateMatch* selector, m_selectors) {
        selector->setMerge(b);
    }
}

} // namespace MainWindow
