/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CATEGORYIMAGECONFIG_H
#define CATEGORYIMAGECONFIG_H

#include <DB/ImageInfoPtr.h>

#include <QDialog>
#include <QImage>
#include <QLabel>

class QComboBox;
class QLabel;

namespace DB
{
class ImageInfo;
}

namespace Viewer
{
class CategoryImageConfig : public QDialog
{
    Q_OBJECT

public:
    static CategoryImageConfig *instance();
    void setCurrentImage(const QImage &image, const DB::ImageInfoPtr &info);
    void show();

protected slots:
    void groupChanged();
    void memberChanged();
    void slotSet();

protected:
    QString currentGroup();

private:
    static CategoryImageConfig *s_instance;
    CategoryImageConfig();
    QComboBox *m_group;
    QStringList m_categoryNames;
    QComboBox *m_member;
    QLabel *m_current;
    QImage m_image;
    QLabel *m_imageLabel;
    DB::ImageInfoPtr m_info;
};
}

#endif /* CATEGORYIMAGECONFIG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
