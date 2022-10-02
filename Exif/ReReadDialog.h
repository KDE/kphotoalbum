// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REREADDIALOG_H
#define REREADDIALOG_H

#include <kpabase/FileNameList.h>

#include <QDialog>

class QCheckBox;
class QLabel;
class QListWidget;

namespace Exif
{

class ReReadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReReadDialog(QWidget *parent);
    // prevent hiding of base class method:
    using QDialog::exec;
    int exec(const DB::FileNameList &);

protected Q_SLOTS:
    void readInfo();
    void warnAboutDates(bool);

private:
    DB::FileNameList m_list;
    QCheckBox *m_exifDB;
    QCheckBox *m_date;
    QCheckBox *m_orientation;
    QCheckBox *m_description;
    QCheckBox *m_force_date;
    QListWidget *m_fileList;
};
}

#endif /* REREADDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
