/* SPDX-FileCopyrightText: 2012-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "Process.h"

#include <QTextStream>

/**
  \class Utilities::Process
  \brief QProcess subclass which collects stdout and print stderr
*/

Utilities::Process::Process(QObject *parent)
    : QProcess(parent)
{
    connect(this, &Process::readyReadStandardError, this, &Process::readStandardError);
    connect(this, &Process::readyReadStandardOutput, this, &Process::readStandardOutput);
}

QString Utilities::Process::stdOut() const
{
    return m_stdout;
}

QString Utilities::Process::stdErr() const
{
    return m_stderr;
}

void Utilities::Process::readStandardError()
{
    setReadChannel(QProcess::StandardError);
    QTextStream stream(this);
    m_stderr.append(stream.readAll());
}

void Utilities::Process::readStandardOutput()
{
    setReadChannel(QProcess::StandardOutput);
    QTextStream stream(this);
    m_stdout.append(stream.readAll());
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_Process.cpp"
