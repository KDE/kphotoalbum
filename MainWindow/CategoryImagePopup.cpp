/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CategoryImagePopup.h"

#include "Window.h"

#include <DB/CategoryCollection.h>
#include <Viewer/CategoryImageConfig.h>
#include <kpabase/StringSet.h>

#include <KLocalizedString>
#include <qstringlist.h>

void MainWindow::CategoryImagePopup::populate(const QImage &image, const DB::FileName &imageName)
{
    clear();

    m_image = image;
    m_imageInfo = DB::ImageDB::instance()->info(imageName);

    // add the categories
    const QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const DB::CategoryPtr &category : categories) {
        if (!category->isSpecialCategory()) {
            bool categoryMenuEnabled = false;
            const QString categoryName = category->name();
            QMenu *categoryMenu = new QMenu(this);
            categoryMenu->setTitle(category->name());

            // add category members
            const Utilities::StringSet members = m_imageInfo->itemsOfCategory(categoryName);
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

#include "moc_CategoryImagePopup.cpp"
