// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIFTREEVIEW_H
#define EXIFTREEVIEW_H

#include <kpabase/StringSet.h>

#include <QTreeWidget>

namespace Exif
{
using Utilities::StringSet;

class TreeView : public QTreeWidget
{
    Q_OBJECT

public:
    TreeView(const QString &title, QWidget *parent);
    StringSet selected();
    void setSelectedExif(const StringSet &selected);
    void reload();

protected Q_SLOTS:
    void toggleChildren(QTreeWidgetItem *);
};

}

#endif /* EXIFTREEVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
