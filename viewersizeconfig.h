#ifndef VIEWERSIZECONFIG_H
#define VIEWERSIZECONFIG_H

#include <qvgroupbox.h>
class QCheckBox;
class QSpinBox;

class ViewerSizeConfig :public QVGroupBox {
    Q_OBJECT

public:
    ViewerSizeConfig( const QString& title, QWidget* parent, const char* name = 0 );
    void setSize( const QSize& size );
    QSize size();
    void setLaunchFullScreen( bool b );
    bool launchFullScreen() const;

private:
    QCheckBox* _fullScreen;
    QSpinBox* _width;
    QSpinBox* _height;
};


#endif /* VIEWERSIZECONFIG_H */

