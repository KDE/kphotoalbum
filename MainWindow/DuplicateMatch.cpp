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

#include "DuplicateMatch.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QImage>
#include <KLocale>
#include <QRadioButton>

namespace MainWindow {

DuplicateMatch::DuplicateMatch(const DB::FileNameList& files )
{
    QVBoxLayout* topLayout = new QVBoxLayout(this);
    QGroupBox* box = new QGroupBox;
    box->setCheckable(true);
    box->setTitle(i18n("Merge"));
    QVBoxLayout* layout = new QVBoxLayout(box);
    topLayout->addWidget(box);

    QPixmap pix = QPixmap::fromImage(QImage(files.first().absolute()).scaled(QSize(300,300),Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QLabel* image = new QLabel;
    image->setPixmap(pix);
    layout->addWidget(image);

    bool first = true;
    Q_FOREACH(const DB::FileName& fileName, files) {
        QRadioButton* button = new QRadioButton(fileName.relative());
        layout->addWidget(button);
        if ( first ) {
            button->setChecked(true);
            first = false;
        }
    }
}

void DuplicateMatch::pixmapLoaded(const DB::FileName &fileName, const QSize &size, const QSize &fullSize, int angle, const QImage &image, const bool loadedOK)
{
}

} // namespace MainWindow
