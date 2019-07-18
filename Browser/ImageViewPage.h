/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
