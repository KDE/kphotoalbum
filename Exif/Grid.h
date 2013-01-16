#ifndef EXIF_GRID_H
#define EXIF_GRID_H

#include <Q3GridView>
#include <DB/FileName.h>
#include <Utilities/Set.h>
#include <QMap>

using Utilities::StringSet;

namespace Exif {

class Grid :public Q3GridView
{
    Q_OBJECT

public:
    explicit Grid( QWidget* parent, const char* name = 0 );
    void setFileName( const DB::FileName& fileName );

signals:
    QString searchStringChanged( const QString& text );

private:
    OVERRIDE void paintCell ( QPainter * p, int row, int col );
    OVERRIDE void resizeEvent( QResizeEvent* );
    OVERRIDE void keyPressEvent( QKeyEvent* );

    StringSet exifGroups( const QMap<QString, QStringList>& exifInfo );
    QMap<QString,QStringList> itemsForGroup( const QString& group, const QMap<QString, QStringList>& exifInfo );
    QString groupName( const QString& exifName );
    QString exifNameNoGroup( const QString& fullName );
    void calculateMaxKeyWidth( const QMap<QString, QStringList>& exifInfo );

private slots:
    void updateGrid();
    void slotCharsetChange( const QString& charset );

private:
    QMap<int, QPair<QString,QStringList> > m_texts;
    QSet<int> m_headers;
    int m_maxKeyWidth;
    QString m_search;
    DB::FileName m_fileName;
};

} // namespace Exif

#endif // EXIF_GRID_H
