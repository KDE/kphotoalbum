/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

namespace RemoteControl
{

class RemoteVideoInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(int imageId READ imageId WRITE setImageId NOTIFY imageIdChanged)
    Q_PROPERTY(double progress READ progress WRITE setProgress NOTIFY progressChanged)

public:
    explicit RemoteVideoInfo(QObject *parent = nullptr);

    bool active() const;
    void setActive(bool newActive);

    const QString &url() const;
    void setUrl(const QString &url);

    int imageId() const;
    void setImageId(int newImageId);

    void setProgress(double progres);
    double progress() const;

signals:
    void activeChanged();
    void urlChanged();
    void imageIdChanged();
    void progressChanged();

private:
    bool m_active = false;
    QString m_url;
    int m_imageId = -1;
    double m_progress = 0;
};

} // namespace RemoteControl
