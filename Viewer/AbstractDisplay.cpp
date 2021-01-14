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

// vi:expandtab:tabstop=4 shiftwidth=4:
