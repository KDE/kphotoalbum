#ifndef EXIFDIALOG_H
#define EXIFDIALOG_H
#include <kdialogbase.h>
#include <qgridview.h>
#include "set.h"
#include "imageclient.h"
class QTable;

namespace Exif
{

class InfoDialog : public KDialogBase, public ImageClient {
    Q_OBJECT

public:
    InfoDialog( const QString& fileName, QWidget* parent, const char* name = 0 );
    virtual QSize sizeHint() const;
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );

protected slots:
    void updateSearchString( const QString& );

private:
    QLabel* _searchLabel;
    QLabel* _pix;
};

class Grid :public QGridView
{
    Q_OBJECT

public:
    Grid( const QString& fileName, QWidget* parent, const char* name = 0 );

signals:
    QString searchStringChanged( const QString& text );

protected:
    virtual void paintCell ( QPainter * p, int row, int col );
    virtual void resizeEvent( QResizeEvent* );
    virtual void keyPressEvent( QKeyEvent* );

    Set<QString> exifGroups( const QMap<QString, QString>& exifInfo );
    QMap<QString,QString> itemsForGroup( const QString& group, const QMap<QString, QString>& exifInfo );
    QString groupName( const QString& exifName );
    QString exifNameNoGroup( const QString& fullName );
    void calculateMaxKeyWidth( const QMap<QString, QString>& exifInfo );

protected slots:
    void updateGrid();

private:
    QMap<int, QPair<QString,QString> > _texts;
    Set<int> _headers;
    int _maxKeyWidth;
    QString _search;
};

}

#endif /* EXIFDIALOG_H */

