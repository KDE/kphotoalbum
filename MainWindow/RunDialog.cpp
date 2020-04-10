/* Copyright (C) 2009-2020 Wes Hardaker <kpa@capturedonearth.com>

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

#include "RunDialog.h"

#include "Window.h"

#include <KLocalizedString>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <krun.h>
#include <kshell.h>

MainWindow::RunDialog::RunDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // xgettext: no-c-format
    QString txt = i18n("<p>Enter your command to run below:</p>"
                       "<p><i>%all will be replaced with a file list</i></p>");
    QLabel *label = new QLabel(txt);
    mainLayout->addWidget(label);

    m_cmd = new QLineEdit();
    mainLayout->addWidget(m_cmd);
    m_cmd->setMinimumWidth(400);
    // xgettext: no-c-format
    txt = i18n("<p>Enter the command you want to run on your image file(s). "
               "KPhotoAlbum will run your command and replace any '%all' tokens "
               "with a list of your files. For example, if you entered:</p>"
               "<ul><li>cp %all /tmp</li></ul>"
               "<p>Then the files you selected would be copied to the /tmp "
               "directory</p>"
               "<p>You can also use %each to have a command be run once per "
               "file.</p>");
    m_cmd->setWhatsThis(txt);
    label->setWhatsThis(txt);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(this, &QDialog::accepted, this, &RunDialog::slotMarkGo);
}

void MainWindow::RunDialog::setImageList(const DB::FileNameList &fileList)
{
    m_fileList = fileList;
}

void MainWindow::RunDialog::slotMarkGo()
{
    QString cmdString = m_cmd->text();
    // xgettext: no-c-format
    QRegExp replaceall = QRegExp(i18nc("As in 'Execute a command and replace any occurrence of %all with the filenames of all selected files'", "%all"));
    // xgettext: no-c-format
    QRegExp replaceeach = QRegExp(i18nc("As in 'Execute a command for each selected file in turn and replace any occurrence of %each with the filename ", "%each"));

    // Replace the %all argument first
    QStringList fileList;
    for (const DB::FileName &fileName : m_fileList)
        fileList.append(fileName.absolute());

    cmdString.replace(replaceall, KShell::joinArgs(fileList));

    if (cmdString.contains(replaceeach)) {
        // cmdString should be run multiple times, once per "each"
        QString cmdOnce;
        for (const DB::FileName &filename : m_fileList) {
            cmdOnce = cmdString;
            cmdOnce.replace(replaceeach, filename.absolute());
            KRun::runCommand(cmdOnce, MainWindow::Window::theMainWindow());
        }
    } else {
        KRun::runCommand(cmdString, MainWindow::Window::theMainWindow());
    }
}

void MainWindow::RunDialog::show()
{
    QDialog::show();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
