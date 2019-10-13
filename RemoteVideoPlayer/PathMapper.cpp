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

#include "PathMapper.h"
#include "PathMappingDialog.h"
#include <QFileInfo>
#include <QSettings>

static QString escape(QString path)
{
    return path.replace("/", "-+-+-+-");
}

static QString unescape(QString path)
{
    return path.replace("-+-+-+-", "/");
}

PathMapper::PathMapper()
{
    QSettings settings("KPhotoAlbum");
    settings.beginGroup("PathMappings");
    for (auto key : settings.childKeys()) {
        m_mappings.append({ unescape(key), settings.value(key).toString() });
    }
}

bool PathMapper::configurePath(const QString &linuxPath, const QString &hostPath)
{
    PathMappingDialog dialog;
    dialog.setLinuxPath(linuxPath);
    dialog.setHostPath(hostPath);

    if (dialog.exec() == QDialog::Accepted) {
        removeMapping(linuxPath, hostPath);
        addMapping(dialog.linuxPath(), dialog.hostPath());
        emit setupChanged();
        return true;
    } else
        return false;
}

PathMapper &PathMapper::instance()
{
    static PathMapper instance;
    return instance;
}

QString PathMapper::map(const QString linuxPath)
{
    for (const Mapping &item : m_mappings) {
        if (linuxPath.startsWith(item.linuxPath))
            return item.hostPath + linuxPath.mid(item.linuxPath.length());
    }

    const QString path = QFileInfo(linuxPath).path();
    bool ok = configurePath(path, path);
    if (ok)
        return map(linuxPath);
    else
        return {};
}

void PathMapper::removeMapping(const QString &linuxPath, const QString &hostPath)
{
    m_mappings.removeOne({ linuxPath, hostPath });
    QSettings settings("KPhotoAlbum");
    settings.beginGroup("PathMappings");
    settings.remove(escape(linuxPath));
    emit setupChanged();
}

void PathMapper::addMapping(const QString &linuxPath, const QString &hostPath)
{
    m_mappings.append({ linuxPath, hostPath });
    QSettings settings("KPhotoAlbum");
    settings.beginGroup("PathMappings");
    settings.setValue(escape(linuxPath), hostPath);
}

QVector<PathMapper::Mapping> PathMapper::mappings() const
{
    return m_mappings;
}
