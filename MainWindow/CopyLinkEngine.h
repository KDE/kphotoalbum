/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COPYLINKENGINE_H
#define COPYLINKENGINE_H

// Qt includes
#include <QObject>

namespace MainWindow
{

class CopyLinkEngine : public QObject
{
    Q_OBJECT

public:
    enum Action {
        Copy,
        Link
    };

    explicit CopyLinkEngine(QObject *parent);

public slots:
    void selectTarget(QWidget *parent, QList<QUrl> &files, Action action);

private: // Variables
    QString m_lastTarget;

};

}

#endif // COPYLINKENGINE_H

// vi:expandtab:tabstop=4 shiftwidth=4:
