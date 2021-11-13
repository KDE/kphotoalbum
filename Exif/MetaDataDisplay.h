// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl@l3u.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef METADATADISPLAY_H
#define METADATADISPLAY_H

#include <QWidget>

class QLabel;

namespace Exif
{

class MetaDataDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit MetaDataDisplay(QWidget *parent = nullptr);
    void setFileName(const QString &fileName);

private: // Functions;
    QLabel *keyLabel(const QString &text);
    QLabel *valueLabel();

private: // Variables
    QLabel *m_absolutePath;
    QLabel *m_size;
    QLabel *m_created;
    QLabel *m_modified;
    QLabel *m_owner;
    QLabel *m_group;
    QLabel *m_permissions;
    QLabel *m_mimeType;

};

}

#endif // METADATADISPLAY_H
