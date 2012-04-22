#ifndef VIDEOSHOOTER_H
#define VIDEOSHOOTER_H

#include <QObject>
#include <DB/ImageInfoPtr.h>

namespace Viewer {
class ViewerWidget;

class VideoShooter : public QObject
{
    Q_OBJECT

public:
    static void go( const DB::ImageInfoPtr& info, Viewer::ViewerWidget* viewer);

private slots:
    void start(const DB::ImageInfoPtr& info, ViewerWidget*);
    void doShoot();

private:
    static VideoShooter* m_instance;
    explicit VideoShooter();
    ViewerWidget* m_viewer;
    bool m_infoboxVisible;
    DB::ImageInfoPtr m_info;
    bool m_wasPlaying;
};

}
#endif // VIDEOSHOOTER_H
