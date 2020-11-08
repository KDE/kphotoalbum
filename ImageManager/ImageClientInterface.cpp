/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageClientInterface.h"

#include "AsyncLoader.h"

ImageManager::ImageClientInterface::~ImageClientInterface()
{
    AsyncLoader::instance()->stop(this);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
