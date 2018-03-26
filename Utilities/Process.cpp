/* Copyright 2012-2018 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QTextStream>

#include "Process.h"

/**
  \class Utilities::Process
  \brief QProcess subclass which collects stdout and print stderr
*/

Utilities::Process::Process(QObject *parent) :
    QProcess(parent)
{
    connect( this, SIGNAL(readyReadStandardError()), this, SLOT(readStandardError()));
    connect( this, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
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
