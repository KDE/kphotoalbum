#ifndef EXIF_GRID_H
#define EXIF_GRID_H

#include <QScrollArea>
#include <DB/FileName.h>
#include <Utilities/Set.h>
#include <QMap>

class QLabel;

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
    OVERRIDE void keyPressEvent( QKeyEvent* );
    OVERRIDE bool eventFilter(QObject*, QEvent*);

    StringSet exifGroups( const QMap<QString, QStringList>& exifInfo );
    QMap<QString,QStringList> itemsForGroup( const QString& group, const QMap<QString, QStringList>& exifInfo );
    QString groupName( const QString& exifName );
    QString exifNameNoGroup( const QString& fullName );
    void scroll(int dy);
    void updateSearch();
    QLabel* headerLabel(const QString& title);
    QPair<QLabel*,QLabel*> infoLabelPair(const QString& title, const QString& value, const QColor& color);

private slots:
    void setupUI( const QString& charset );
    void updateWidgetSize();

private:
    QList< QPair<QLabel*,QLabel*> > m_labels;
    int m_maxKeyWidth;
    QString m_search;
    DB::FileName m_fileName;
};

} // namespace Exif

#endif // EXIF_GRID_H
