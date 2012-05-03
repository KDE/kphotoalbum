#include "Process.h"
#include <KDebug>

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

QString Utilities::Process::stdout() const
{
    return m_stdout;
}

void Utilities::Process::readStandardError()
{
    setReadChannel(QProcess::StandardError);
    QTextStream stream(this);
    kWarning() << stream.readAll();
}

void Utilities::Process::readStandardOutput()
{
    setReadChannel(QProcess::StandardOutput);
    QTextStream stream(this);
    m_stdout.append(stream.readAll());
}
