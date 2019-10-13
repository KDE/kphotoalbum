/* Copyright (C) 2003-2019 Jesper K. Pedersen <blackie@kde.org>

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

#pragma once

#include <QObject>
#include <QVector>

class PathMapper : public QObject
{
    Q_OBJECT

public:
    static PathMapper &instance();
    QString map(const QString linuxPath);
    void removeMapping(const QString &linuxPath, const QString &hostPath);
    void addMapping(const QString &linuxPath, const QString &hostPath);
    bool configurePath(const QString &linuxPath, const QString &hostPath);

    struct Mapping {
        QString linuxPath;
        QString hostPath;
        bool operator==(const Mapping &other)
        {
            return other.linuxPath == linuxPath && other.hostPath == hostPath;
        }
    };
    QVector<Mapping> mappings() const;

signals:
    void setupChanged();

private:
    PathMapper();

    QVector<Mapping> m_mappings;
};
