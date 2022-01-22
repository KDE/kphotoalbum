#pragma once

#include <QObject>
class QTimer;

class SlideShow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit SlideShow(QObject *parent = nullptr);

    bool running() const;
    void setRunning(bool newRunning);

signals:
    void runningChanged();
    void requestNext();

private:
    bool m_running = false;
    QTimer *m_timer;
};
