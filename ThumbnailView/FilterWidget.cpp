/* Copyright (C) 2019 The KPhotoAlbum Development Team

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License or (at your option) version 3 or any later version
  accepted by the membership of KDE e. V. (or its successor approved
  by the membership of KDE e. V.), which shall act as a proxy
  defined in Section 14 of version 3 of the license.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
  */

#include "FilterWidget.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KRatingWidget>
#include <QLabel>

ThumbnailView::FilterWidget::FilterWidget(QWidget *parent)
    : KToolBar(parent)
{
    m_actions = new KActionCollection(this);
    m_toggleFilter = addAction(
        QIcon::fromTheme(QLatin1String("view-filter")),
        i18nc("The action enables/disables filtering of images in the thumbnail view.", "Toggle filter"));
    m_toggleFilter->setCheckable(true);
    m_toggleFilter->setToolTip(xi18n("Press <shortcut>Escape</shortcut> to clear filter."));
    connect(m_toggleFilter, &QAction::toggled, this, &FilterWidget::filterToggled);
    m_actions->addAction(QString::fromLatin1("FilterWidget/toggle"), m_toggleFilter);

    m_rating = new KRatingWidget;
    addWidget(m_rating);
    for (short i = 1; i <= 5; i++) {
        QAction *ratingAction = new QAction(i18np("Filter view by rating: %1 star", "Filter view by rating: %1 stars", i));
        m_actions->addAction(QString::fromLatin1("FilterWidget/rating/%1").arg(i), ratingAction);
        m_actions->setDefaultShortcut(ratingAction, Qt::ALT + (Qt::Key_0 + i));
        connect(ratingAction, &QAction::triggered, this, [=]() {
            short rating = i * 2;
            if (static_cast<short>(m_rating->rating()) == rating)
                rating = -1;
            emit ratingChanged(rating);
        });
    }

    m_label = new QLabel;
    resetLabelText();
    addWidget(m_label);

    addActions(m_actions->actions());
    // Note(jzarl): new style connect seems to be confused by overloaded signal in KRatingWidget
    // -> fall back to old-style
    connect(m_rating, SIGNAL(ratingChanged(int)), this, SLOT(slotRatingChanged(int)));
}

void ThumbnailView::FilterWidget::setFilter(const DB::ImageSearchInfo &filter)
{
    // prevent ratingChanged signal when the filter has changed
    blockSignals(true);
    m_rating->setRating(qMax(static_cast<short int>(0), filter.rating()));
    if (filter.isNull()) {
        m_toggleFilter->setChecked(false);
        resetLabelText();
    } else {
        m_toggleFilter->setChecked(true);
        m_label->setText(i18nc("The label gives a textual description of the active filter", "Filter: %1", filter.toString()));
    }
    blockSignals(false);
}

void ThumbnailView::FilterWidget::setEnabled(bool enabled)
{
    m_toggleFilter->setEnabled(enabled);
    m_rating->setEnabled(enabled);
    if (enabled)
        resetLabelText();
    else
        m_label->clear();
}

void ThumbnailView::FilterWidget::slotRatingChanged(int rating)
{
    Q_ASSERT(-1 <= rating && rating <= 10);
    emit ratingChanged((short)rating);
}

void ThumbnailView::FilterWidget::resetLabelText()
{
    m_label->setText(xi18n("Tip: Use <shortcut>Alt+Shift+<placeholder>A-Z</placeholder></shortcut> to toggle a filter for that token."));
}

KActionCollection *ThumbnailView::FilterWidget::actions() const
{
    return m_actions;
}
