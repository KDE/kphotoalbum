#ifndef VIDEOTHUMBNAILCYCLER_H
#define VIDEOTHUMBNAILCYCLER_H

#include <QObject>
#include <DB/Id.h>

class QTimer;

namespace ThumbnailView {

class VideoThumbnailCycler : public QObject
{
    Q_OBJECT
public:
    explicit VideoThumbnailCycler(QObject *parent = 0);
    void setActiveId( const DB::Id& id );

private slots:
    void updateThumbnail();

private:
    void resetPreviousThumbail();
    bool isVideo( const DB::Id& id ) const;
    QString fileNameForId( const DB::Id& ) const;
    void startCycle();
    void stopCycle();

    DB::Id m_id;
    int m_index;
    QTimer* m_timer;
};

}
#endif // VIDEOTHUMBNAILCYCLER_H
