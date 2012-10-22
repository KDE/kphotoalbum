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
#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QImage>

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
    findDuplicates();
}

DuplicateMerger::~DuplicateMerger()
{
    delete ui;
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
    const DB::FileNameList values = m_matches[md5];
    QGroupBox* box = new QGroupBox;
    box->setCheckable(true);
    box->setTitle(i18n("Merge"));
    QVBoxLayout* layout = new QVBoxLayout(box);
    m_topLayout->addWidget(box);

    QPixmap pix = QPixmap::fromImage(QImage(values.first().absolute()).scaled(QSize(300,300),Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QLabel* image = new QLabel;
    image->setPixmap(pix);
    layout->addWidget(image);

    bool first = true;
    Q_FOREACH(const DB::FileName& fileName, values) {
        QRadioButton* button = new QRadioButton(fileName.relative());
        layout->addWidget(button);
        if ( first ) {
            button->setChecked(true);
            first = false;
        }
    }
}

} // namespace MainWindow
