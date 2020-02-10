/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CategoryImagePopup.h"

#include "Window.h"

#include <DB/CategoryCollection.h>
#include <Utilities/StringSet.h>
#include <Viewer/CategoryImageConfig.h>

#include <KLocalizedString>
#include <qstringlist.h>

void MainWindow::CategoryImagePopup::populate(const QImage &image, const DB::FileName &imageName)
{
    clear();

    m_image = image;
    m_imageInfo = DB::ImageDB::instance()->info(imageName);

    // add the categories
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const DB::CategoryPtr category : categories) {
        if (!category->isSpecialCategory()) {
            bool categoryMenuEnabled = false;
            const QString categoryName = category->name();
            QMenu *categoryMenu = new QMenu(this);
            categoryMenu->setTitle(category->name());

            // add category members
            Utilities::StringSet members = m_imageInfo->itemsOfCategory(categoryName);
            for (const QString &member : members) {
                QAction *action = categoryMenu->addAction(member);
                action->setObjectName(categoryName);
                action->setData(member);
                categoryMenuEnabled = true;
            }

            categoryMenu->setEnabled(categoryMenuEnabled);
            addMenu(categoryMenu);
        }
    }

    // Add the Category Editor menu item
    QAction *action = addAction(QString::fromLatin1("viewer-show-category-editor"), this, SLOT(makeCategoryImage()));
    action->setText(i18n("Show Category Editor"));
}

void MainWindow::CategoryImagePopup::slotExecuteService(QAction *action)
{
    QString categoryName = action->objectName();
    QString memberName = action->data().toString();
    if (categoryName.isNull())
        return;
    DB::ImageDB::instance()->categoryCollection()->categoryForName(categoryName)->setCategoryImage(categoryName, memberName, m_image);
}

void MainWindow::CategoryImagePopup::makeCategoryImage()
{
    Viewer::CategoryImageConfig::instance()->setCurrentImage(m_image, m_imageInfo);
    Viewer::CategoryImageConfig::instance()->show();
}

MainWindow::CategoryImagePopup::CategoryImagePopup(QWidget *parent)
    : QMenu(parent)
{
    setTitle(i18n("Make Category Image"));
    connect(this, &CategoryImagePopup::triggered, this, &CategoryImagePopup::slotExecuteService);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
