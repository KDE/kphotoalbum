// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H
#include <kpabase/FileNameList.h>

#include <QDialog>
#include <QLabel>
#include <kjob.h>
#include <qradiobutton.h>

class QLabel;
class QCheckBox;
class KJob;
namespace KIO
{
class Job;
}
namespace MainWindow
{

class DeleteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteDialog(QWidget *parent);
    // prevent hiding of base class method:
    using QDialog::exec;
    int exec(const DB::FileNameList &list);

protected Q_SLOTS:
    void deleteImages();

private:
    DB::FileNameList m_list;
    QLabel *m_label;
    QRadioButton *m_deleteFile;
    QRadioButton *m_useTrash;
    QRadioButton *m_deleteFromDb;
};
}

#endif /* DELETEDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
