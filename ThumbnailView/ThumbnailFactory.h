/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef THUMBNAILFACTORY_H
#define THUMBNAILFACTORY_H

namespace ThumbnailView
{
class ThumbnailWidget;
class CellGeometry;
class ThumbnailModel;

class ThumbnailFactory
{
public:
    virtual ~ThumbnailFactory() { };
    virtual ThumbnailModel *model() = 0;
    virtual CellGeometry *cellGeometry() = 0;
    virtual ThumbnailWidget *widget() = 0;
};

}

#endif /* THUMBNAILFACTORY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
