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

namespace MainWindow {

DuplicateMerger::DuplicateMerger(QWidget *parent) :
    KDialog(parent),
    ui(new Ui::DuplicateMerger)
{
    resize(800,600);

    QWidget* top = new QWidget;
    ui->setupUi(top);
    setMainWidget(top);

    findDuplicates();
}

DuplicateMerger::~DuplicateMerger()
{
    delete ui;
}

void DuplicateMerger::findDuplicates()
{
    QMap<DB::MD5, DB::FileNameList> map;

    Q_FOREACH( const DB::FileName& fileName, DB::ImageDB::instance()->images() ) {
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const DB::MD5 md5 = info->MD5Sum();
        map[md5].append(fileName);
    }

    Q_FOREACH( const DB::MD5& md5, map.keys() ) {
        const DB::FileNameList values = map[md5];
        if ( values.count() > 1 ) {
            ui->text->append(i18n("==%1==").arg(md5.toHexString()));
            Q_FOREACH(const DB::FileName& fileName, values) {
                ui->text->append( fileName.relative() );
            }
        }
    }
}

} // namespace MainWindow
