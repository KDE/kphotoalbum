// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2006-2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007-2011 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RAWIMAGEDECODER_H
#define RAWIMAGEDECODER_H

#include "ImageDecoder.h"

#include <kpabase/FileName.h>

#include <QStringList>

namespace ImageManager
{

class RAWImageDecoder : public ImageDecoder
{
public:
protected:
    bool _decode(QImage *img, ImageRequest *request, QSize *fullSize, int dim = -1) override;
    bool _mightDecode(const DB::FileName &imageFile) override;
};
}

#endif /* RAWIMAGEDECODER_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
