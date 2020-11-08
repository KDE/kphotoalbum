/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEVIEWPAGE_H
#define IMAGEVIEWPAGE_H
#include "BrowserPage.h"

#include <DB/FileName.h>
#include <DB/ImageSearchInfo.h>

namespace Browser
{

/**
 * \brief The page showing the actual images.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class ImageViewPage : public BrowserPage
{
public:
    ImageViewPage(const DB::ImageSearchInfo &info, BrowserWidget *browser);
    ImageViewPage(const DB::FileName &context, BrowserWidget *browser);
    void activate() override;
    Viewer viewer() override;
    bool isSearchable() const override;
    bool showDuringMovement() const override;
    Breadcrumb breadcrumb() const override;

private:
    DB::FileName m_context;
};

}

#endif /* IMAGEVIEWPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
