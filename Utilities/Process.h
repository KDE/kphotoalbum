#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>

namespace Utilities
{

class Process : public QProcess
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = 0);
    QString stdout() const;
    
private slots:
    void readStandardError();
    void readStandardOutput();

private:
    QString m_stdout;
};

}

#endif // PROCESS_H
