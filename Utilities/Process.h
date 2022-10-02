// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>

namespace Utilities
{

class Process : public QProcess
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = nullptr);
    QString stdOut() const;
    QString stdErr() const;

private Q_SLOTS:
    void readStandardError();
    void readStandardOutput();

private:
    QString m_stdout;
    QString m_stderr;
};

}

#endif // PROCESS_H
// vi:expandtab:tabstop=4 shiftwidth=4:
