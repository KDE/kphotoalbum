// SPDX-FileCopyrightText: 2019 - 2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "FilterWidget.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KRatingWidget>
#include <QLabel>

ThumbnailView::FilterWidget::FilterWidget(QWidget *parent)
    : KToolBar(parent)
{
    setWindowTitle(i18nc("Name/title of the filter toolbar widget", "Filter Toolbar"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("view-filter")));

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
        m_actions->setDefaultShortcut(ratingAction, QKeySequence(Qt::ALT | (Qt::Key_0 + i)));
        connect(ratingAction, &QAction::triggered, this, [=, this]() {
            short rating = i * 2;
            if (static_cast<short>(m_rating->rating()) == rating)
                rating = -1;
            Q_EMIT ratingChanged(rating);
        });
        m_rating->addAction(ratingAction);
    }
    m_actions->readSettings();

    m_label = new QLabel;
    resetLabelText();
    addWidget(m_label);

    connect(m_rating, QOverload<int>::of(&KRatingWidget::ratingChanged), this, &FilterWidget::slotRatingChanged);
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
    Q_EMIT ratingChanged((short)rating);
}

void ThumbnailView::FilterWidget::resetLabelText()
{
    m_label->setText(xi18n("Tip: Use <shortcut>Alt+Shift+<placeholder>A-Z</placeholder></shortcut> to toggle a filter for that token."));
}

KActionCollection *ThumbnailView::FilterWidget::actions() const
{
    return m_actions;
}

#include "moc_FilterWidget.cpp"
