#ifndef EXIFDIALOG_H
#define EXIFDIALOG_H
#include <kdialogbase.h>
#include <qgridview.h>
#include "set.h"
class QTable;

class ExifDialog : public KDialogBase {
    Q_OBJECT

public:
    ExifDialog( const QString& fileName, QWidget* parent, const char* name = 0 );
    virtual QSize sizeHint() const;
};

class ExifGrid :public QGridView
{
    Q_OBJECT

public:
    ExifGrid( const QString& fileName, QWidget* parent, const char* name = 0 );

protected:
    virtual void paintCell ( QPainter * p, int row, int col );
    virtual void resizeEvent( QResizeEvent* );

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
};


#endif /* EXIFDIALOG_H */

