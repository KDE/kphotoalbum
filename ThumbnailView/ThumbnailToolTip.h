/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef THUMBNAILTOOLTIP_H
#define THUMBNAILTOOLTIP_H
#include <ImageManager/ImageClientInterface.h>
#include <Utilities/ToolTip.h>
#include <kpabase/FileName.h>

#include <QEvent>
#include <qlabel.h>
#include <qtimer.h>

namespace DB
{
class ImageInfo;
}

namespace ThumbnailView
{
class ThumbnailWidget;

class ThumbnailToolTip : public Utilities::ToolTip
{
    Q_OBJECT

public:
    explicit ThumbnailToolTip(ThumbnailWidget *view);
    virtual void setActive(bool);

private slots:
    void requestToolTip();

private:
    bool eventFilter(QObject *, QEvent *e) override;
    void placeWindow() override;

private:
    ThumbnailWidget *m_view;
    bool m_widthInverse;
    bool m_heightInverse;
};
}

#endif /* THUMBNAILTOOLTIP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
