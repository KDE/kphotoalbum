/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeleteDialog.h"

#include <Utilities/DeleteFiles.h>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

using namespace MainWindow;

DeleteDialog::DeleteDialog(QWidget *parent)
    : QDialog(parent)
    , m_list()
{
    setWindowTitle(i18nc("@title:window", "Removing Items"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *top = new QWidget;
    QVBoxLayout *lay1 = new QVBoxLayout(top);
    mainLayout->addWidget(top);

    m_label = new QLabel;
    lay1->addWidget(m_label);

    m_useTrash = new QRadioButton;
    lay1->addWidget(m_useTrash);

    m_deleteFile = new QRadioButton;
    lay1->addWidget(m_deleteFile);

    m_deleteFromDb = new QRadioButton;
    lay1->addWidget(m_deleteFromDb);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DeleteDialog::deleteImages);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DeleteDialog::reject);
    mainLayout->addWidget(buttonBox);
}

int DeleteDialog::exec(const DB::FileNameList &list)
{
    if (!list.size())
        return 0;

    bool someFileExists = false;
    for (const DB::FileName &file : list) {
        if (file.exists()) {
            someFileExists = true;
            break;
        }
    }

    const QString msg1 = i18np("Removing 1 item", "Removing %1 items", list.size());
    const QString msg2 = i18np("Selected item will be removed from the database.<br/>What do you want to do with the file on disk?",
                               "Selected %1 items will be removed from the database.<br/>What do you want to do with the files on disk?",
                               list.size());

    const QString txt = QString::fromLatin1("<p><b><center><font size=\"+3\">%1</font><br/>%2</center></b></p>").arg(msg1, msg2);

    m_useTrash->setText(i18np("Move file to Trash", "Move %1 files to Trash", list.size()));
    m_deleteFile->setText(i18np("Delete file from disk", "Delete %1 files from disk", list.size()));
    m_deleteFromDb->setText(i18np("Only remove the item from database", "Only remove %1 items from database", list.size()));

    m_label->setText(txt);
    m_list = list;

    // disable trash/delete options if files don't exist
    m_useTrash->setChecked(someFileExists);
    m_useTrash->setEnabled(someFileExists);
    m_deleteFile->setEnabled(someFileExists);
    m_deleteFromDb->setChecked(!someFileExists);

    return QDialog::exec();
}

void DeleteDialog::deleteImages()
{
    bool anyDeleted = Utilities::DeleteFiles::deleteFiles(m_list, m_deleteFile->isChecked() ? Utilities::DeleteFromDisk : m_useTrash->isChecked() ? Utilities::MoveToTrash
                                                                                                                                                  : Utilities::BlockFromDatabase);
    if (anyDeleted)
        accept();
    else
        reject();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DeleteDialog.cpp"
