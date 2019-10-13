#include "MainWindow.h"
#include <QApplication>

#include <QFileInfo>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow window;
    window.show();

    return a.exec();
}
