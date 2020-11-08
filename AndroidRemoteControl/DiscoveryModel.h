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

#ifndef REMOTECONTROL_DISCOVERYMODEL_H
#define REMOTECONTROL_DISCOVERYMODEL_H

#include "ThumbnailModel.h"

namespace RemoteControl
{

class DiscoverAction;

class DiscoveryModel : public ThumbnailModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

public:
    DiscoveryModel(QObject *parent);
    int count() const;
    void setImages(const QList<int> &images) override;
    void setCurrentSelection(const QList<int> &selection, const QList<int> &allImages);
    void setCurrentAction(DiscoverAction *action);

public slots:
    void setCount(int arg);
    void resetImages();

signals:
    void countChanged();

private:
    int m_count = 0;
    DiscoverAction *m_action = nullptr;
    QList<int> m_allImages;
};

} // namespace RemoteControl

Q_DECLARE_METATYPE(RemoteControl::DiscoveryModel *);

#endif // REMOTECONTROL_DISCOVERYMODEL_H
