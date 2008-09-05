#ifndef VISIBLEOPTIONSMENU_H
#define VISIBLEOPTIONSMENU_H

#include <QList>
#include <QMap>
#include <QMenu>
class KToggleAction;
class KActionCollection;

namespace Viewer {

class VisibleOptionsMenu :public QMenu
{
Q_OBJECT
public:
    VisibleOptionsMenu( QWidget* parent, KActionCollection* actions);

signals:
    void visibleOptionsChanged();

private slots:
    void updateState();
    void toggleShowInfoBox( bool );
    void toggleShowCategory( bool );
    void toggleShowLabel( bool );
    void toggleShowDescription( bool );
    void toggleShowDate( bool );
    void toggleShowTime( bool );
    void toggleShowFilename( bool );
    void toggleShowEXIF( bool );
    void toggleShowImageSize( bool );

private:
    KToggleAction* _showInfoBox;
    KToggleAction* _showLabel;
    KToggleAction* _showDescription;
    KToggleAction* _showDate;
    KToggleAction* _showTime;
    KToggleAction* _showFileName;
    KToggleAction* _showExif;
    KToggleAction* _showImageSize;
    QList<KToggleAction*> _actionList;
};

}

#endif /* VISIBLEOPTIONSMENU_H */

