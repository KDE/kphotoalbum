/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONTROL_SETTINGS_H
#define REMOTECONTROL_SETTINGS_H

#include <QColor>
#include <QObject>

namespace RemoteControl
{

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int thumbnailSize READ thumbnailSize WRITE setThumbnailSize NOTIFY thumbnailSizeChanged)
    Q_PROPERTY(int categoryItemSize READ categoryItemSize WRITE setCategoryItemSize NOTIFY categoryItemSizeChanged)
    Q_PROPERTY(double overviewIconSize READ overviewIconSize WRITE setOverviewIconSize NOTIFY overviewIconSizeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY foregroundColorChanged)

    double m_thumbnailScale;

public:
    static Settings &instance();
    int thumbnailSize() const;
    void setThumbnailSize(int size);
    int categoryItemSize() const;
    double overviewIconSize() const;

    QColor backgroundColor() const;

    QColor textColor() const;

public slots:
    void setCategoryItemSize(int size);
    void setOverviewIconSize(double size);

signals:
    void thumbnailSizeChanged();
    void categoryItemSizeChanged();
    void overviewIconSizeChanged();
    void backgroundColorChanged();
    void foregroundColorChanged();

private:
    explicit Settings() = default;
    int m_categoryItemSize;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SETTINGS_H
