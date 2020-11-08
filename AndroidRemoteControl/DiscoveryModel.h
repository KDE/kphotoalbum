/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
