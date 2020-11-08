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

#ifndef REMOTECONTROL_IMAGEDETAILS_H
#define REMOTECONTROL_IMAGEDETAILS_H

#include "RemoteCommand.h"
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

public slots:
    void clear();
    void setData(const ImageDetailsResult &data);
    QStringList itemsOfCategory(const QString &category);
    QString dummy() const { return {}; }
    QString age(const QString &category, const QString &item);

signals:
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
