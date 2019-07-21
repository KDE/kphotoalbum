/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef EXIF_GRID_H
#define EXIF_GRID_H

#include <DB/FileName.h>
#include <Utilities/StringSet.h>

#include <QMap>
#include <QScrollArea>

class QLabel;

using Utilities::StringSet;

namespace Exif
{

class Grid : public QScrollArea
{
    Q_OBJECT

public:
    explicit Grid(QWidget *parent);
    void setFileName(const DB::FileName &fileName);

public slots:
    void updateSearchString(const QString &);

private:
    void keyPressEvent(QKeyEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

    StringSet exifGroups(const QMap<QString, QStringList> &exifInfo);
    QMap<QString, QStringList> itemsForGroup(const QString &group, const QMap<QString, QStringList> &exifInfo);
    QString groupName(const QString &exifName);
    QString exifNameNoGroup(const QString &fullName);
    void scroll(int dy);
    QLabel *headerLabel(const QString &title);
    QPair<QLabel *, QLabel *> infoLabelPair(const QString &title, const QString &value, const QColor &color);

private slots:
    void setupUI(const QString &charset);
    void updateWidgetSize();

private:
    QList<QPair<QLabel *, QLabel *>> m_labels;
    int m_maxKeyWidth;
    DB::FileName m_fileName;
};

} // namespace Exif

#endif // EXIF_GRID_H
// vi:expandtab:tabstop=4 shiftwidth=4:
