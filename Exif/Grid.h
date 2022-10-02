// SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIF_GRID_H
#define EXIF_GRID_H

#include <kpabase/FileName.h>
#include <kpabase/StringSet.h>

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

public Q_SLOTS:
    void updateSearchString(const QString &);
    /**
     * @brief setupUI sets up the scroll area for the given charset.
     * Usually, this slot is only indirectly called through the setFileName function,
     * but calling it directly can be used to change the display character set without changing the file name.
     * @param charset
     */
    void setupUI(const QString &charset);

private:
    void keyPressEvent(QKeyEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

    StringSet exifGroups(const QMap<QString, QStringList> &exifInfo);
    QMap<QString, QStringList> itemsForGroup(const QString &group, const QMap<QString, QStringList> &exifInfo);
    QString groupName(const QString &exifName);
    QString exifNameNoGroup(const QString &fullName);
    void scroll(int dy);
    QLabel *headerLabel(const QString &title);
    QPair<QLabel *, QLabel *> infoLabelPair(const QString &title, const QString &value, const QPalette::ColorRole role);

private Q_SLOTS:
    void updateWidgetSize();

private:
    QList<QPair<QLabel *, QLabel *>> m_labels;
    DB::FileName m_fileName;
};

} // namespace Exif

#endif // EXIF_GRID_H
// vi:expandtab:tabstop=4 shiftwidth=4:
