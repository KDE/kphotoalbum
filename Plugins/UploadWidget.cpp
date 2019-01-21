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

#include "UploadWidget.h"

#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QTreeView>

#include <Settings/SettingsData.h>

#include "ImageCollection.h"
#include "UploadImageCollection.h"

namespace Plugins {

UploadWidget::UploadWidget( QWidget* parent )
    : KIPI::UploadWidget( parent )
{
    QTreeView* listView = new QTreeView(this);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(listView);

    m_model = new QFileSystemModel(this);
    m_model->setFilter( QDir::Dirs | QDir::NoDotDot);
    listView->setModel(m_model);
    m_path = Settings::SettingsData::instance()->imageDirectory();
    const QModelIndex index = m_model->setRootPath( m_path );
    listView->setRootIndex(index);
    connect(listView, &QTreeView::activated, this, &UploadWidget::newIndexSelected);
}

KIPI::ImageCollection UploadWidget::selectedImageCollection() const
{
    return KIPI::ImageCollection( new Plugins::UploadImageCollection( m_path ) );
}

void UploadWidget::newIndexSelected(const QModelIndex& index )
{
    m_path = m_model->filePath(index);
}


} // namespace Plugins
// vi:expandtab:tabstop=4 shiftwidth=4:
