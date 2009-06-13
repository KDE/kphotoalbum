#ifndef CENTERINGICONVIEW_H
#define CENTERINGICONVIEW_H
#include <QListView>
class QTimer;

namespace Browser
{

/**
 * \brief QListView subclass which shows its content as icons centered in the middle
 *
 * To make KPhotoAlbum slightly more sexy, the overview page has its items
 * centered at the middle of the screen. This class has the capability to
 * do that when \ref setMode is called with \ref CenterView as argument.
 */
class CenteringIconView :public QListView
{
    Q_OBJECT
public:
    enum ViewMode {CenterView, NormalIconView };

    CenteringIconView( QWidget* parent );
    void setViewMode( ViewMode );
    OVERRIDE void setModel( QAbstractItemModel* );
    OVERRIDE void showEvent( QShowEvent* );

protected:
    OVERRIDE void resizeEvent( QResizeEvent* );

private slots:
    void setupMargin();

private:
    QTimer* _resizeTimer;

private:
    ViewMode _viewMode;
};

}


#endif /* CENTERINGICONVIEW_H */

