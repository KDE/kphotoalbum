// SPDX-FileCopyrightText: 2021-2023 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

public Q_SLOTS:
    void selectTarget(QWidget *parent, const QList<QUrl> &files, Action action);

private: // Variables
    QString m_lastTarget;
};

}

#endif // COPYLINKENGINE_H

// vi:expandtab:tabstop=4 shiftwidth=4:
