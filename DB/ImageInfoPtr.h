/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef IMAGEINFOPTR_H
#define IMAGEINFOPTR_H
#include <QExplicitlySharedDataPointer>

namespace DB
{
class ImageInfo;
using ImageInfoPtr = QExplicitlySharedDataPointer<ImageInfo>;
}

#endif /* IMAGEINFOPTR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
