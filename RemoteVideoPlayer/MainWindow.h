#pragma once

#include "Executor.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static void addMessage(const QString &message);

private:
    void populateMappings();

    Ui::MainWindow *ui;
    Executor *m_executor = nullptr;
    static MainWindow *m_instance;
};
