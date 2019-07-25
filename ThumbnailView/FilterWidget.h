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

    KActionCollection *actions() const;

signals:
    void filterToggled(bool enabled);
    void ratingChanged(short rating);

public slots:
    void setFilter(const DB::ImageSearchInfo &filter);
    /**
     * @brief setEnabled enables or disables the filter controls.
     * If the ThumbnailView is not active, setEnable should be set to \c false.
     * @param enabled
     */
    void setEnabled(bool enabled);

protected slots:
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
