#ifndef SPEEDDISPLAY_H
#define SPEEDDISPLAY_H
#include <qdialog.h>
class QTimer;
class QLabel;
class QHBoxLayout;

class SpeedDisplay :public QDialog {
    Q_OBJECT

public:
    SpeedDisplay( QWidget* parent, const char* name = 0 );
    void display( int );
    void start();
    void end();
    void go();

private:
    QTimer* _timer;
    QLabel* _label;
    QHBoxLayout* _layout;
};


#endif /* SPEEDDISPLAY_H */

