/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#include "TreeView.h"

#include "Info.h"

#include <Utilities/StringSet.h>

#include <qmap.h>
#include <qstringlist.h>

using Utilities::StringSet;

Exif::TreeView::TreeView(const QString &title, QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderLabel(title);
    reload();
    connect(this, &TreeView::itemClicked, this, &TreeView::toggleChildren);
}

void Exif::TreeView::toggleChildren(QTreeWidgetItem *parent)
{
    if (!parent)
        return;

    bool on = parent->checkState(0) == Qt::Checked;
    for (int index = 0; index < parent->childCount(); ++index) {
        parent->child(index)->setCheckState(0, on ? Qt::Checked : Qt::Unchecked);
        toggleChildren(parent->child(index));
    }
}

StringSet Exif::TreeView::selected()
{
    StringSet result;
    for (QTreeWidgetItemIterator it(this); *it; ++it) {
        if ((*it)->checkState(0) == Qt::Checked)
            result.insert((*it)->text(1));
    }
    return result;
}

void Exif::TreeView::setSelectedExif(const StringSet &selected)
{
    for (QTreeWidgetItemIterator it(this); *it; ++it) {
        bool on = selected.contains((*it)->text(1));
        (*it)->setCheckState(0, on ? Qt::Checked : Qt::Unchecked);
    }
}

void Exif::TreeView::reload()
{
    clear();
    setRootIsDecorated(true);
    QStringList keys = Exif::Info::instance()->availableKeys().toList();
    keys.sort();

    QMap<QString, QTreeWidgetItem *> tree;

    for (QStringList::const_iterator keysIt = keys.constBegin(); keysIt != keys.constEnd(); ++keysIt) {
        QStringList subKeys = (*keysIt).split(QLatin1String("."));
        QTreeWidgetItem *parent = nullptr;
        QString path;
        for (const QString &subKey : subKeys) {
            if (!path.isEmpty())
                path += QString::fromLatin1(".");
            path += subKey;
            if (tree.contains(path))
                parent = tree[path];
            else {
                if (parent == nullptr)
                    parent = new QTreeWidgetItem(this, QStringList(subKey));
                else
                    parent = new QTreeWidgetItem(parent, QStringList(subKey));
                parent->setText(1, path); // This is simply to make the implementation of selected easier.
                parent->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                parent->setCheckState(0, Qt::Unchecked);
                tree.insert(path, parent);
            }
        }
    }

    if (QTreeWidgetItem *item = topLevelItem(0))
        item->setExpanded(true);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
