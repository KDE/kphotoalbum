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

#include <KIO/CopyJob>
#include <KLocalizedString>

#include <QFileDialog>
#include <QPushButton>

MainWindow::CopyPopup::CopyPopup(
        QWidget *parent,
        DB::ImageInfoPtr current,
        DB::FileNameList imageList,
        CopyType copyType
        ) : QMenu(parent)
{
    connect(this, &CopyPopup::triggered, this, &CopyPopup::slotCopy);

    m_list = imageList;
    m_currentInfo = current;

    QAction *action;
    if (copyType == Copy) {
        setTitle(i18n("Copy image(s) to..."));
        action = addAction(i18n("Copy currently selected image to..."));
        action->setData(QString::fromLatin1("current"));

        action = addAction(i18n("Copy all selected images to..."));
        action->setData(QString::fromLatin1("all"));
        if (m_list.size() == 1) {
            action->setEnabled(false);
        }
    } else {
        setTitle(i18n("Link image(s) to..."));
        action = addAction(i18n("Link currently selected image to..."));
        action->setData(QString::fromLatin1("linkCurrent"));

        action = addAction(i18n("Link all selected images to..."));
        action->setData(QString::fromLatin1("linkAll"));
        if (m_list.size() == 1) {
            action->setEnabled(false);
        }
    }
}

MainWindow::CopyPopup::~CopyPopup()
{
}

void MainWindow::CopyPopup::slotCopy(QAction *action)
{
    QString mode = action->data().toString();

    QList<QUrl> src;

    if (mode == QString::fromLatin1("current") || mode == QString::fromLatin1("linkCurrent")) {
        src << QUrl::fromLocalFile(m_currentInfo->fileName().absolute());
    } else {
        QStringList srcList = m_list.toStringList(DB::AbsolutePath);
        for (int i = 0; i < srcList.size(); ++i) {
            src << QUrl::fromLocalFile(srcList.at(i));
        }
    }

    Q_ASSERT( src.size()>0 );
    QFileDialog dialog(this);
    dialog.setDirectoryUrl( src.at(0));
    dialog.setLabelText( QFileDialog::Accept, i18nc("@action:button", "Copy"));

    if (mode == QString::fromLatin1("current") || mode == QString::fromLatin1("linkCurrent")) {
        if (mode == QString::fromLatin1("current"))
            dialog.setWindowTitle(i18nc("@title:window", "Copy image to..."));
        else
            dialog.setWindowTitle(i18nc("@title:window", "Link image to..."));
        dialog.selectUrl(src[0]);
        dialog.setFileMode( QFileDialog::ExistingFile );
    } else {
        if (mode == QString::fromLatin1("all"))
            dialog.setWindowTitle(i18nc("@title:window", "Copy images to..."));
        else
            dialog.setWindowTitle(i18nc("@title:window", "Link images to..."));
        dialog.setFileMode( QFileDialog::ExistingFile );
        dialog.setOption( QFileDialog::ShowDirsOnly, true);
    }

    if (! dialog.exec()) {
        return;
    }

    if (mode == QString::fromLatin1("current") || mode == QString::fromLatin1("all"))
        KIO::copy(src, dialog.selectedUrls().at(0));
    else
        KIO::link(src, dialog.selectedUrls().at(0));
}

#include "CopyPopup.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
