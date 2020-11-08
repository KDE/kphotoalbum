/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "RemoteCommand.h"

namespace RemoteControl
{

using RoleMap = QHash<int, QByteArray>;
class CategoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasData READ hasData NOTIFY hasDataChanged)

public:
    enum { NameRole,
           IconRole,
           EnabledRole,
           TypeRole };
    explicit CategoryModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    RoleMap roleNames() const override;
    void setCategories(const QList<Category> &);
    bool hasData() const;

signals:
    void hasDataChanged();

private:
    QList<Category> m_categories;
};

}
Q_DECLARE_METATYPE(RemoteControl::CategoryModel *);

#endif // CATEGORYMODEL_H
