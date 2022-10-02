// SPDX-FileCopyrightText: 2019-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <DB/ImageSearchInfo.h>

#include <KToolBar>

class KActionCollection;
class KRatingWidget;
class QAction;
class QLabel;
class QWidget;

namespace ThumbnailView
{
/**
 * @brief The FilterWidget class provides a KToolBar widget to interact with the thumbnail filter.
 * You can use it to set the rating filter, and it gives some visual feedback when the filter changes.
 *
 * \image html FilterWidget.png "FilterWidget when no filter is set"
 */
class FilterWidget : public KToolBar
{
    Q_OBJECT
public:
    explicit FilterWidget(QWidget *parent = nullptr);

    /**
     * @brief actions contains shortcut actions for setting the rating filter.
     * @see ThumbnailFacade::actions()
     * @return the QActions for this widget.
     */
    KActionCollection *actions() const;

Q_SIGNALS:
    void filterToggled(bool enabled);
    void ratingChanged(short rating);

public Q_SLOTS:
    void setFilter(const DB::ImageSearchInfo &filter);
    /**
     * @brief setEnabled enables or disables the filter controls.
     * If the ThumbnailView is not active, setEnable should be set to \c false.
     * @param enabled
     */
    void setEnabled(bool enabled);

protected Q_SLOTS:
    void slotRatingChanged(int rating);
    void resetLabelText();

private:
    KActionCollection *m_actions;
    QAction *m_toggleFilter;
    KRatingWidget *m_rating;
    QLabel *m_label;
};
}

#endif // FILTERWIDGET_H
