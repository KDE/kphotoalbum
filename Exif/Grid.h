#ifndef EXIF_GRID_H
#define EXIF_GRID_H

#include <QScrollArea>
#include <DB/FileName.h>
#include <Utilities/Set.h>
#include <QMap>

using Utilities::StringSet;

namespace Exif {

class Grid :public QScrollArea
{
    Q_OBJECT

public:
    explicit Grid( QWidget* parent );
    void setFileName( const DB::FileName& fileName );

signals:
    QString searchStringChanged( const QString& text );

private:
    void paintCell ( QPainter * p, int row, int col );
    OVERRIDE void resizeEvent( QResizeEvent* );
    OVERRIDE void keyPressEvent( QKeyEvent* );
    OVERRIDE bool eventFilter(QObject*, QEvent*);

    StringSet exifGroups( const QMap<QString, QStringList>& exifInfo );
    QMap<QString,QStringList> itemsForGroup( const QString& group, const QMap<QString, QStringList>& exifInfo );
    QString groupName( const QString& exifName );
    QString exifNameNoGroup( const QString& fullName );
    void calculateMaxKeyWidth( const QMap<QString, QStringList>& exifInfo );
    void scroll(int dy);

private slots:
    void updateGrid();
    void setupUI( const QString& charset );
    void updateWidgetSize();

private:
    QMap<int, QPair<QString,QStringList> > m_texts;
    QSet<int> m_headers;
    int m_maxKeyWidth;
    QString m_search;
    DB::FileName m_fileName;
};

} // namespace Exif

#endif // EXIF_GRID_H
