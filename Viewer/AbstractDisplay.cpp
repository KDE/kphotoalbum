/* SPDX-FileCopyrightText: 2003-2018 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AbstractDisplay.h"

#include <DB/ImageInfo.h>
#include <kpabase/SettingsData.h>

Viewer::AbstractDisplay::AbstractDisplay(QWidget *parent)
    : QWidget(parent)
    , m_info(nullptr)
{
}

bool Viewer::AbstractDisplay::setImage(DB::ImageInfoPtr info, bool forward)
{
    m_info = info;
    return setImageImpl(info, forward);
}

#include "moc_AbstractDisplay.cpp"
// vi:expandtab:tabstop=4 shiftwidth=4:
