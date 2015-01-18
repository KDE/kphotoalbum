/* Copyright (C) 2003-2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "InfoBox.h"

// Qt includes
#include <QApplication>
#include <QToolButton>
#include <QCursor>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QBitmap>
#include <QDebug>
#include <QScrollBar>
#include <qglobal.h>

// KDE includes
#include <KActionCollection>
#include <KGlobal>
#include <KIconLoader>
#include <KDebug>

// Local includes
#include "Browser/BrowserWidget.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include "MainWindow/Window.h"
#include "VisibleOptionsMenu.h"

#ifdef HAVE_KGEOMAP
#include <QDialog>
#include <QVBoxLayout>
#include "Map/MapView.h"
#endif

using namespace Settings;

Viewer::InfoBox::InfoBox(Viewer::ViewerWidget* viewer) : KTextBrowser(viewer)
    ,m_viewer(viewer)
    ,m_hoveringOverLink(false)
    ,m_infoBoxResizer(this)
    ,m_menu(nullptr)
#ifdef HAVE_KGEOMAP
    ,m_map(nullptr)
#endif
{
    setFrameStyle(Box | Plain);
    setLineWidth(1);
    setMidLineWidth(0);
    setAutoFillBackground(false);

    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(0, 0, 0, 170)); // r, g, b, A
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Link, QColor(Qt::blue).light());
    setPalette(p);

    m_jumpToContext = new QToolButton(this);
    m_jumpToContext->setIcon(KIcon(QString::fromUtf8("kphotoalbum")));
    m_jumpToContext->setFixedSize(16, 16);
    connect(m_jumpToContext, SIGNAL(clicked()), this, SLOT(jumpToContext()));
    connect(this, SIGNAL(highlighted(QString)), SLOT(linkHovered(QString)));
    m_jumpToContext->setCursor(Qt::ArrowCursor);

#ifdef HAVE_KGEOMAP
    m_showOnMap = new QToolButton(this);
    m_showOnMap->setIcon(KIcon(QString::fromUtf8("edit-web-search")));
    m_showOnMap->setFixedSize(16, 16);
    m_showOnMap->setCursor(Qt::ArrowCursor);
    m_showOnMap->setToolTip(i18n("Show the geographic position of this image on a map"));
    connect(m_showOnMap, SIGNAL(clicked()), this, SLOT(launchMapView()));
    m_showOnMap->hide();

    connect(m_viewer, SIGNAL(soughtTo(DB::FileName)),
            this, SLOT(updateMapForCurrentImage(DB::FileName)));
#endif

    KRatingWidget* rating = new KRatingWidget( nullptr );

    // Unfortunately, the KRatingWidget now thinks that it has some absurdly big
    // dimensions. This call will persuade it to stay reasonably small.
    QPixmap::grabWidget(rating);

    for (int i = 0; i <= 10; ++i) {
        rating->setRating( i );
        // Workaround for http://trolltech.no/developer/task-tracker/index_html?method=entry&id=142869
        // There's no real transparency in grabWidget() :(
        QPixmap pixmap = QPixmap::grabWidget(rating);
        pixmap.setMask(pixmap.createHeuristicMask());
        m_ratingPixmap.append(pixmap);
    }

    delete rating;
}

QVariant Viewer::InfoBox::loadResource(int type, const QUrl& name)
{
    if (name.scheme() == QString::fromUtf8("KRatingWidget")) {
        short int rating = name.host().toShort();
        return m_ratingPixmap[rating];
    }
    return KTextBrowser::loadResource(type, name);
}

void Viewer::InfoBox::setSource(const QUrl& source)
{
    int index = source.path().toInt();
    QPair<QString,QString> p = m_linkMap[index];
    QString category = p.first;
    QString value = p.second;
    Browser::BrowserWidget::instance()->load(category, value);
    showBrowser();
}

void Viewer::InfoBox::setInfo(const QString& text, const QMap<int, QPair<QString,QString>>& linkMap)
{
    m_linkMap = linkMap;
    setText(text);

    hackLinkColorForQt44();

#ifdef HAVE_KGEOMAP
    if (m_viewer->currentInfo()->coordinates().hasCoordinates()) {
        m_showOnMap->show();
    } else {
        m_showOnMap->hide();
    }
#endif

    setSize();
}

void Viewer::InfoBox::setSize()
{
    const int maxWidth = Settings::SettingsData::instance()->infoBoxWidth();
    const int maxHeight = Settings::SettingsData::instance()->infoBoxHeight();

    document()->setPageSize(QSize(maxWidth, maxHeight));
    bool showVerticalBar = document()->size().height() > maxHeight;

    setVerticalScrollBarPolicy(showVerticalBar ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    const int realWidth = static_cast<int>(document()->idealWidth())
                          + (showVerticalBar ? verticalScrollBar()->width() + frameWidth() : 0)
                          + m_jumpToContext->width() + 10;

    resize(realWidth, qMin((int) document()->size().height(), maxHeight));
}

void Viewer::InfoBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        possiblyStartResize(event->pos());
    }
    KTextBrowser::mousePressEvent(event);
}

void Viewer::InfoBox::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_infoBoxResizer.isActive()) {
        Settings::SettingsData::instance()->setInfoBoxWidth(width());
        Settings::SettingsData::instance()->setInfoBoxHeight(height());
    }

    m_infoBoxResizer.deactivate();
    KTextBrowser::mouseReleaseEvent(event);
}

void Viewer::InfoBox::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_infoBoxResizer.isActive()) {
            m_infoBoxResizer.setPos(event->pos());
        } else {
            m_viewer->infoBoxMove();
        }
        // Do not tell KTextBrowser about the mouse movement, as this will just start a selection.
    } else {
        updateCursor(event->pos());
        KTextBrowser::mouseMoveEvent(event);
    }
}

void Viewer::InfoBox::linkHovered(const QString& linkName)
{
    if (linkName.isEmpty()) {
        emit noTagHovered();
    } else {
        emit tagHovered(m_linkMap[linkName.toInt()]);
    }

    m_hoveringOverLink = ! linkName.isNull();
}

void Viewer::InfoBox::jumpToContext()
{
    Browser::BrowserWidget::instance()->addImageView(m_viewer->currentInfo()->fileName());
    showBrowser();
}

void Viewer::InfoBox::showBrowser()
{
    QDesktopWidget* desktop = qApp->desktop();
    if (desktop->screenNumber(Browser::BrowserWidget::instance()) == desktop->screenNumber(m_viewer)) {
        if (m_viewer->showingFullScreen()) {
            m_viewer->setShowFullScreen(false);
        }
        MainWindow::Window::theMainWindow()->raise();
    }
}

/**
 * Update the cursor based on the cursors position in the info box
 */
void Viewer::InfoBox::updateCursor(const QPoint& pos)
{
    const int border = 25;

    bool left = (pos.x() < border);
    bool right = pos.x() > width() - border;
    bool top = pos.y() < border;
    bool bottom = pos.y() > height() - border;

    Settings::Position windowPos = Settings::SettingsData::instance()->infoBoxPosition();

    Qt::CursorShape shape = Qt::SizeAllCursor;
    if (m_hoveringOverLink) {
        shape = Qt::PointingHandCursor;
    } else if (atBlackoutPos(left, right, top, bottom, windowPos)) {
        shape = Qt::SizeAllCursor;
    } else if ((left && top) || (right && bottom)) {
        shape = Qt::SizeFDiagCursor;
    } else if ((left && bottom) || (right && top)) {
        shape = Qt::SizeBDiagCursor;
    } else if (top || bottom) {
        shape = Qt::SizeVerCursor;
    } else if (left || right) {
        shape = Qt::SizeHorCursor;
    }

    setCursor(shape);
    viewport()->setCursor(shape);
}

/**
 * Return true if we are at an edge of the image info box that is towards the edge of the viewer.
 * We can forexample not make the box taller at the bottom if the box is sitting at the bottom of the viewer.
 */
bool Viewer::InfoBox::atBlackoutPos(bool left, bool right, bool top, bool bottom, Settings::Position pos) const
{
    return (left && (pos == Left || pos == TopLeft || pos == BottomLeft))
           || (right && (pos == Right || pos == TopRight || pos == BottomRight))
           || (top && (pos == Top || pos == TopLeft || pos == TopRight))
           || (bottom && (pos == Bottom || pos == BottomLeft || pos == BottomRight));
}

void Viewer::InfoBox::possiblyStartResize(const QPoint& pos)
{
    const int border = 25;

    bool left = (pos.x() < border);
    bool right = pos.x() > width() - border;
    bool top = pos.y() < border;
    bool bottom = pos.y() > height() - border;

    if (left || right || top || bottom) {
        m_infoBoxResizer.setup(left, right, top, bottom);
    }
}

void Viewer::InfoBox::resizeEvent(QResizeEvent*)
{
    QPoint pos = viewport()->rect().adjusted(0, 2, -m_jumpToContext->width() - 2, 0).topRight();
    m_jumpToContext->move(pos);
#ifdef HAVE_KGEOMAP
    pos.setY(pos.y() + 20);
    m_showOnMap->move(pos);
#endif
}

void Viewer::InfoBox::hackLinkColorForQt44()
{
    QTextCursor cursor(document());
    Q_FOREVER {
        QTextCharFormat f = cursor.charFormat();
        if (f.isAnchor()) {
            f.setForeground(QColor(Qt::blue).light());
            QTextCursor c2 = cursor;
            c2.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            c2.setCharFormat(f);
        }
        if (cursor.atEnd()) {
            break;
        }
        cursor.movePosition(QTextCursor::NextCharacter);
    }
}

void Viewer::InfoBox::contextMenuEvent(QContextMenuEvent* event)
{
    if (! m_menu) {
        m_menu = new VisibleOptionsMenu(m_viewer, new KActionCollection((QObject*) nullptr));
        connect(m_menu, SIGNAL(visibleOptionsChanged()), m_viewer, SLOT(updateInfoBox()));
    }
    m_menu->exec(event->globalPos());
}

#ifdef HAVE_KGEOMAP
void Viewer::InfoBox::launchMapView()
{
    if (! m_map) {
        m_map = new Map::MapView(m_viewer, Map::MapView::MapViewWindow);
    }

    m_map->addImage(m_viewer->currentInfo());
    m_map->setShowThumbnails(false);
    m_map->zoomToMarkers();
    m_map->show();
    m_map->raise();
}

void Viewer::InfoBox::updateMapForCurrentImage(DB::FileName)
{
    if (! m_map) {
        return;
    }

    if (m_viewer->currentInfo()->coordinates().hasCoordinates()) {
        m_map->displayStatus(Map::MapView::MapStatus::ImageHasCoordinates);
        m_map->clear();
        m_map->addImage(m_viewer->currentInfo());
        m_map->setCenter(m_viewer->currentInfo());
    } else {
        m_map->displayStatus(Map::MapView::MapStatus::ImageHasNoCoordinates);
    }
}
#endif

#include "InfoBox.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
