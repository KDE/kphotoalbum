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
#include <QScrollBar>

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
    m_labels.clear();
    Background* widget = new Background;

    QGridLayout* layout = new QGridLayout(widget);
    layout->setSpacing(0);
    int row = 0;

    const QMap<QString,QStringList> map = Exif::Info::instance()->infoForDialog( m_fileName, charset );
    const StringSet groups = exifGroups( map );

    Q_FOREACH( const QString& group, groups ) {
        layout->addWidget(headerLabel(group),row++,0,1,4);

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
            m_labels.append( qMakePair(keyLabel,valueLabel));
        }
        ++row;
    }

    setWidget(widget);
    widget->show();
    QTimer::singleShot(0, this, SLOT(updateWidgetSize()));
}

QLabel *Exif::Grid::headerLabel(const QString &title)
{
    QLabel* label = new QLabel(title);

    QPalette pal;
    pal.setBrush(QPalette::Background, Qt::lightGray);
    label->setPalette(pal);
    label->setAutoFillBackground(true);
    label->setAlignment(Qt::AlignCenter);

    return label;
}

void Exif::Grid::updateWidgetSize()
{
    widget()->setFixedSize(viewport()->width(), widget()->height());
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

void Exif::Grid::scroll(int dy)
{
    verticalScrollBar()->setValue(verticalScrollBar()->value()+dy);
}

void Exif::Grid::updateSearch()
{
    QPair<QLabel*,QLabel*> tuple;
    Q_FOREACH( tuple, m_labels ) {
        const bool matches = tuple.first->text().contains( m_search, Qt::CaseInsensitive ) && m_search.length() != 0;
        QPalette pal = tuple.first->palette();
        pal.setBrush(QPalette::Foreground, matches ? Qt::red : Qt::black);
        tuple.first->setPalette(pal);
        tuple.second->setPalette(pal);
        QFont fnt = tuple.first->font();
        fnt.setBold(matches);
        tuple.first->setFont(fnt);
        tuple.second->setFont(fnt);
    }
}

void Exif::Grid::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() ) {
    case Qt::Key_Down:
        scroll( 20 );
        return;
    case Qt::Key_Up:
        scroll( -20 );
        return;
    case Qt::Key_PageDown:
        scroll( viewport()->height() - 20);
        return;
    case Qt::Key_PageUp:
        scroll(-(viewport()->height()- 20));
        return;
    case Qt::Key_Backspace:
        m_search.remove( m_search.length()-1, 1 );
        emit searchStringChanged( m_search );
        updateSearch();
        return;
    case Qt::Key_Escape:
        QScrollArea::keyPressEvent( e ); // Propagate to close dialog.
        return;
    }

    if ( !e->text().isEmpty() ) {
        m_search += e->text();
        emit searchStringChanged( m_search );
        updateSearch();
    }
}

bool Exif::Grid::eventFilter(QObject* object, QEvent* event)
{
    if ( object == viewport() && event->type() == QEvent::Resize) {
        QResizeEvent* re = static_cast<QResizeEvent*>(event);
        widget()->setFixedSize(re->size().width(), widget()->height());
    }
    return false;
}

void Exif::Grid::setFileName(const DB::FileName &fileName)
{
    m_fileName = fileName;
    setupUI( Settings::SettingsData::instance()->iptcCharset() );
}
