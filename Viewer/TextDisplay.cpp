/* SPDX-FileCopyrightText: 2007-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TextDisplay.h"

#include "ImageDisplay.h"

#include <DB/ImageDB.h>

#include <QLabel>
#include <QVBoxLayout>
#include <qlabel.h>
#include <qlayout.h>

/**
 * Display a text instead of actual image/video data.
 */

Viewer::TextDisplay::TextDisplay(QWidget *parent)
    : AbstractDisplay(parent)
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    m_text = new QLabel(this);
    lay->addWidget(m_text);
    m_text->setAlignment(Qt::AlignCenter);
}

bool Viewer::TextDisplay::setImageImpl(DB::ImageInfoPtr info, bool forward)
{
    Q_UNUSED(info)
    Q_UNUSED(forward)
    return true;
}

void Viewer::TextDisplay::setText(const QString text)
{
    m_text->setText(text);
}

bool Viewer::TextDisplay::canRotate()
{
    return false;
}

bool Viewer::TextDisplay::canZoom()
{
    return false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TextDisplay.cpp"
