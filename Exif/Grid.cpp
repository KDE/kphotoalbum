#include "Grid.h"
#include "Info.h"
#include <QResizeEvent>
#include <QPainter>
#include <QTimer>
#include <QGridLayout>
#include <QLabel>
#include <Settings/SettingsData.h>
#include <QDebug>
#include <QColor>

Exif::Grid::Grid( QWidget* parent )
    :QScrollArea( parent )
{
    setFocusPolicy( Qt::WheelFocus );
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->installEventFilter(this);
    setMinimumSize(800,400);
}

class Background :public QWidget {
protected:
    OVERRIDE void paintEvent(QPaintEvent* event) {
        QPainter painter(this);
        painter.fillRect( event->rect(), QColor(Qt::white));
    }
};

void Exif::Grid::setupUI( const QString& charset )
{
    delete this->widget();
    Background* widget = new Background;

    QGridLayout* layout = new QGridLayout(widget);
    layout->setSpacing(0);
    int row = 0;

    QMap<QString,QStringList> map = Exif::Info::instance()->infoForDialog( m_fileName, charset );
    calculateMaxKeyWidth( map );
    StringSet groups = exifGroups( map );
    Q_FOREACH( const QString& group, groups ) {
        QLabel* label = new QLabel(group);

        QPalette pal;
        pal.setBrush(QPalette::Background, Qt::lightGray);
        label->setPalette(pal);
        label->setAutoFillBackground(true);
        label->setAlignment(Qt::AlignCenter);

        layout->addWidget(label,row++,0,1,4);

        int col = -1;
        // Items of group
        const QMap<QString,QStringList> items = itemsForGroup( group, map );
        QStringList sorted = items.keys();
        sorted.sort();
        Q_FOREACH( const QString& key, sorted ) {
            QLabel* keyLabel = new QLabel( exifNameNoGroup( key ) );
            QLabel* valueLabel = new QLabel(items[key].join( QLatin1String(", ")));
            col = (col +1) % 4;
            if ( col == 0 )
                ++row;
            layout->addWidget(keyLabel, row, col);
            layout->addWidget(valueLabel,row,++col);

            QPalette pal;
            const int index = row * 2 + col;
            pal.setBrush(QPalette::Background, (index % 4 == 0 || index % 4 == 3)? Qt::white : QColor(226, 235, 250));
            keyLabel->setPalette(pal);
            valueLabel->setPalette(pal);
            keyLabel->setAutoFillBackground(true);
            valueLabel->setAutoFillBackground(true);
        }
        ++row;
    }

    setWidget(widget);
    widget->show();
    QTimer::singleShot(0, this, SLOT(updateWidgetSize()));
}

void Exif::Grid::updateWidgetSize()
{
    widget()->resize(viewport()->width(), widget()->height());
}

void Exif::Grid::paintCell( QPainter * p, int row, int col )
{
#if 0
    int index = row * 2 + col;
    QColor background;
    bool isHeader = m_headers.contains( 2* (index / 2) );
    if ( isHeader )
        background = Qt::lightGray;
    else
        background = (index % 4 == 0 || index % 4 == 3) ? Qt::white : QColor(226, 235, 250);

    p->fillRect( cellRect(), background );

    if ( isHeader ) {
        p->drawText( cellRect(), ((index % 2) ? Qt::AlignLeft : Qt::AlignRight ), m_texts[index].first );
    }
    else {
        QString text = m_texts[index].first;
        bool match = ( !m_search.isEmpty() && text.contains( m_search, Qt::CaseInsensitive ) );
        QFont f(p->font());
        f.setWeight( match ? QFont::Bold : QFont::Normal );
        p->setFont( f );
        p->setPen( match ? Qt::red : Qt::black );
        p->drawText( cellRect(), Qt::AlignLeft, text);
        QRect rect = cellRect();
        rect.setX( m_maxKeyWidth + 10 );
        p->drawText( rect, Qt::AlignLeft, m_texts[index].second.join( QString::fromAscii(", ") ) );
    }
#endif
}


StringSet Exif::Grid::exifGroups( const QMap<QString,QStringList>& exifInfo )
{
    StringSet result;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        result.insert( groupName( it.key() ) );
    }
    return result;
}

QMap<QString,QStringList> Exif::Grid::itemsForGroup( const QString& group, const QMap<QString, QStringList>& exifInfo )
{
    QMap<QString,QStringList> result;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        if ( groupName( it.key() ) == group )
            result.insert( it.key(), it.value() );
    }
    return result;
}

QString Exif::Grid::groupName( const QString& exifName )
{
    QStringList list = exifName.split(QString::fromLatin1("."));
    list.pop_back();
    return list.join( QString::fromLatin1(".") );
}

QString Exif::Grid::exifNameNoGroup( const QString& fullName )
{
    return fullName.split(QString::fromLatin1(".")).last();
}

void Exif::Grid::resizeEvent( QResizeEvent* )
{
    QTimer::singleShot( 0, this, SLOT( updateGrid() ) );
}

void Exif::Grid::updateGrid()
{
#if 0
    setCellWidth( clipper()->width() / 2 );
#endif
}

void Exif::Grid::calculateMaxKeyWidth( const QMap<QString, QStringList>& exifInfo )
{
    QFont f = font();
    f.setWeight( QFont::Bold );
    QFontMetrics metrics( f );
    m_maxKeyWidth = 0;
    for( QMap<QString,QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it ) {
        m_maxKeyWidth = qMax( m_maxKeyWidth, metrics.width( exifNameNoGroup( it.key() ) ) );
    }
}

void Exif::Grid::keyPressEvent( QKeyEvent* e )
{
#if 0
    switch ( e->key() ) {
    case Qt::Key_Down:
        scrollBy( 0, cellHeight() );
        return;
    case Qt::Key_Up:
        scrollBy( 0, -cellHeight() );
        return;
    case Qt::Key_PageDown:
        scrollBy( 0, (clipper()->height() - cellHeight() ));
        return;
    case Qt::Key_PageUp:
        scrollBy( 0, -(clipper()->height() - cellHeight()) );
        return;
    case Qt::Key_Backspace:
        m_search.remove( m_search.length()-1, 1 );
        emit searchStringChanged( m_search );
        updateContents();
        return;
    case Qt::Key_Escape:
        Q3GridView::keyPressEvent( e ); // Propagate to close dialog.
        return;
    }

    if ( !e->text().isEmpty() ) {
        m_search += e->text();
        emit searchStringChanged( m_search );
        updateContents();
    }
#endif
}

bool Exif::Grid::eventFilter(QObject* object, QEvent* event)
{
    if ( object == viewport() && event->type() == QEvent::Resize) {
        QResizeEvent* re = static_cast<QResizeEvent*>(event);
        widget()->resize(re->size().width(), widget()->height());
    }
    return false;
}

void Exif::Grid::setFileName(const DB::FileName &fileName)
{
    m_fileName = fileName;
    setupUI( Settings::SettingsData::instance()->iptcCharset() );
}
