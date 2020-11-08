/* SPDX-FileCopyrightText: 2014-2016 Tobias Leupold <tobias.leupold@web.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COPYPOPUP_H
#define COPYPOPUP_H

// Qt includes
#include <QList>
#include <QMenu>
#include <QUrl>

namespace MainWindow
{

class CopyPopup : public QMenu
{
    Q_OBJECT

public:
    enum CopyType {
        Copy,
        Link
    };

    enum CopyAction {
        CopyCurrent,
        CopyAll,
        LinkCurrent,
        LinkAll
    };

    explicit CopyPopup(QWidget *parent,
                       QUrl &selectedFile,
                       QList<QUrl> &allSelectedFiles,
                       QString &lastTarget,
                       CopyType copyType);

private slots:
    void slotCopy(QAction *action);

private:
    QUrl &m_selectedFile;
    QList<QUrl> &m_allSelectedFiles;
    QString &m_lastTarget;
};

}

#endif // COPYPOPUP_H

// vi:expandtab:tabstop=4 shiftwidth=4:
