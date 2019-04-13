/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H
#include "Breadcrumb.h"
#include "BrowserPage.h"
#include <DB/CategoryPtr.h>
#include <QAbstractListModel>

namespace AnnotationDialog { class Dialog; }
namespace DB { class ImageSearchInfo; class MediaCount; }
namespace Browser {

class BrowserWidget;

/**
 * \brief The overview page in the browser (the one containing People, Places, Show Images etc)
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * The OverviewPage implements two interfaces \ref BrowserPage (with
 * information about the page itself) and QAbstractListModel (the model
 * set on the view in the Browser).
 *
 * Combining both in the same class was done mostly for convenience, the
 * two interfaces was to a large extend referring to the same data.
 */
class OverviewPage :public QAbstractListModel, public BrowserPage
{
public:
    OverviewPage( const Breadcrumb& breadcrumb, const DB::ImageSearchInfo& info, Browser::BrowserWidget* );
    int rowCount ( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data ( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    void activate() override;
    BrowserPage* activateChild( const QModelIndex& ) override;
    Qt::ItemFlags flags ( const QModelIndex & index ) const override;
    bool isSearchable() const override;
    Breadcrumb breadcrumb() const override;
    bool showDuringMovement() const override;


private:
    /**
     * @brief Count images/videos in each category.
     */
    void updateImageCount();
    QList<DB::CategoryPtr> categories() const;

    bool isCategoryIndex( int row ) const;
    bool isGeoPositionIndex( int row ) const;
    bool isExivIndex( int row ) const;
    bool isSearchIndex( int row ) const;
    bool isUntaggedImagesIndex( int row ) const;
    bool isImageIndex( int row ) const;

    QVariant categoryInfo( int row, int role ) const;
    QVariant geoPositionInfo( int role ) const;
    QVariant exivInfo( int role ) const;
    QVariant searchInfo( int role ) const;
    QVariant untaggedImagesInfo( int rolw ) const;
    QVariant imageInfo( int role ) const;

    BrowserPage* activateExivAction();
    BrowserPage* activateSearchAction();
    BrowserPage* activateUntaggedImagesAction();

private:
    QMap<int,bool> m_rowHasSubcategories;
    static AnnotationDialog::Dialog* s_config;
    Breadcrumb m_breadcrumb;
};

}


#endif /* OVERVIEWPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
