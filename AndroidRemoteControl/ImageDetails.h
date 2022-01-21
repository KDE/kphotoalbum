// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTECONTROL_IMAGEDETAILS_H
#define REMOTECONTROL_IMAGEDETAILS_H

#include "../RemoteControl/RemoteCommand.h"
#include <QObject>
namespace RemoteControl
{

class ImageDetails : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName MEMBER m_fileName NOTIFY updated)
    Q_PROPERTY(QString date MEMBER m_date NOTIFY updated)
    Q_PROPERTY(QString description MEMBER m_description NOTIFY updated)
    Q_PROPERTY(QStringList categories READ categories NOTIFY updated)

    // This is just a dummy property to ensure that categories are updated too when changed.
    Q_PROPERTY(QString dummy READ dummy NOTIFY updated)

public:
    static ImageDetails &instance();
    QStringList categories() const;

public Q_SLOTS:
    void clear();
    void setData(const ImageDetailsResult &data);
    QStringList itemsOfCategory(const QString &category);
    QString dummy() const { return {}; }
    QString age(const QString &category, const QString &item);

Q_SIGNALS:
    void updated();

private:
    ImageDetails() = default;
    QString m_fileName;
    QString m_date;
    QString m_description;
    QMap<QString, CategoryItemDetailsList> m_categories;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGEDETAILS_H
