/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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

#include "CopyPopup.h"
#include <klocale.h>
#include <KFileDialog>
#include <KPushButton>
#include <kio/copyjob.h>

MainWindow::CopyPopup::CopyPopup(
    QWidget *parent,
    DB::ImageInfoPtr current,
    DB::FileNameList imageList
) : QMenu(parent)
{
    setTitle(i18n("Copy image(s) to..."));
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(slotCopy(QAction*)));

    m_list = imageList;
    m_currentInfo = current;

    QAction *action;
    action = addAction(i18n("Copy currently selected image to..."));
    action->setData(QString::fromLatin1("current"));

    action = addAction(i18n("Copy all selected images to..."));
    action->setData(QString::fromLatin1("all"));
    if (m_list.size() == 1) {
        action->setEnabled(false);
    }
}

MainWindow::CopyPopup::~CopyPopup()
{
}

void MainWindow::CopyPopup::slotCopy(QAction *action)
{
    QString mode = action->data().toString();

    KUrl::List src;

    if (mode == QString::fromLatin1("current")) {
        src << KUrl::fromPath(m_currentInfo->fileName().absolute());
    } else {
        QStringList srcList = m_list.toStringList(DB::AbsolutePath);
        for (int i = 0; i < srcList.size(); ++i) {
            src << KUrl::fromPath(srcList.at(i));
        }
    }

    // "kfiledialog:///copyTo" -> use last directory that was used in this dialog
    KFileDialog dialog(KUrl("kfiledialog:///copyTo"), QString() /* empty filter */, this);
    dialog.okButton()->setText(i18nc("@action:button", "Copy"));

    if (mode == QString::fromLatin1("current")) {
        dialog.setCaption(i18nc("@title:window", "Copy image to..."));
        dialog.setSelection(src[0].fileName());
        dialog.setMode(KFile::File | KFile::Directory);
    } else {
        dialog.setCaption(i18nc("@title:window", "Copy images to..."));
        dialog.setMode(KFile::Directory);
    }

    if (! dialog.exec()) {
        return;
    }

    KIO::copy(src, dialog.selectedUrl());
}

#include "CopyPopup.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
