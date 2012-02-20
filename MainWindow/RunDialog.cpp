/* Copyright (C) 2009-2010 Wes Hardaker <kpa@capturedonearth.com>

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
#include <KDialog>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <MainWindow/Window.h>
#include <klocale.h>
#include <krun.h>
#include <kshell.h>

MainWindow::RunDialog::RunDialog( QWidget* parent )
    : KDialog( parent )
{
    QWidget* top = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout( top );
    setMainWidget(top);

    // xgettext: no-c-format
    QString txt = i18n("<p>Enter your command to run below:</p>"
                       "<p><i>%all will be replaced with a file list</i></p>");
    QLabel* label = new QLabel(txt);
    layout->addWidget(label);

    _cmd = new KLineEdit();
    layout->addWidget(_cmd);
    _cmd->setMinimumWidth(400);
    // xgettext: no-c-format
    txt = i18n("<p>Enter the command you want to run on your image file(s).  "
               "KPhotoAlbum will run your command and replace any '%all' tokens "
               "with a list of your files.  For example, if you entered:</p>"
               "<ul><li>cp %all /tmp</li></ul>"
               "<p>Then the files you selected would be copied to the /tmp "
               "directory</p>"
               "<p>You can also use %each to have a command be run once per "
               "file.</p>");
    _cmd->setWhatsThis(txt);
    label->setWhatsThis(txt);

    connect( this, SIGNAL( okClicked() ), this, SLOT( slotMarkGo() ) );
}

void MainWindow::RunDialog::setImageList( QStringList fileList )
{
    _fileList = fileList;
}

void MainWindow::RunDialog::slotMarkGo( )
{
    QString cmdString = _cmd->text();
    // xgettext: no-c-format
    QRegExp replaceall = QRegExp(i18n("%all"));
    // xgettext: no-c-format
    QRegExp replaceeach = QRegExp(i18n("%each"));

    // Replace the %all argument first
    cmdString.replace(replaceall, KShell::joinArgs(_fileList));

    if (cmdString.contains(replaceeach)) {
        // cmdString should be run multiple times, once per "each"
        QString cmdOnce;
        for( QStringList::Iterator it = _fileList.begin(); it != _fileList.end(); ++it ) {
            cmdOnce = cmdString;
            cmdOnce.replace(replaceeach, *it);
            KRun::runCommand(cmdOnce, MainWindow::Window::theMainWindow());
        }
    } else {
        KRun::runCommand(cmdString, MainWindow::Window::theMainWindow());
    }
}


void MainWindow::RunDialog::show()
{
    KDialog::show();
}

