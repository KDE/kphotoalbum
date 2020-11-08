/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ToolTip.h"

#include "DescriptionUtil.h"

#include <DB/ImageDB.h>
#include <ImageManager/AsyncLoader.h>
#include <ImageManager/ImageRequest.h>
#include <Settings/SettingsData.h>

#include <QEvent>
#include <QTemporaryFile>

namespace Utilities
{

ToolTip::ToolTip(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
    , m_tmpFileForThumbnailView(nullptr)
{
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setLineWidth(1);
    setMargin(1);

    setWindowOpacity(0.8);
    setAutoFillBackground(true);
    updatePalette();
}

void ToolTip::updatePalette()
{
    QPalette p = palette();
    QColor bgColor = palette().shadow().color();
    bgColor.setAlpha(170);
    p.setColor(QPalette::Background, bgColor);
    p.setColor(QPalette::WindowText, palette().brightText().color());
    setPalette(p);
    // re-enable palette-propagation:
    setAttribute(Qt::WA_SetPalette);
}

void ToolTip::requestImage(const DB::FileName &fileName)
{
    int size = Settings::SettingsData::instance()->previewSize();
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
    if (size != 0) {
        ImageManager::ImageRequest *request = new ImageManager::ImageRequest(fileName, QSize(size, size), info->angle(), this);
        request->setPriority(ImageManager::Viewer);
        ImageManager::AsyncLoader::instance()->load(request);
    } else
        renderToolTip();
}

void ToolTip::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    const DB::FileName fileName = request->databaseFileName();

    delete m_tmpFileForThumbnailView;
    m_tmpFileForThumbnailView = new QTemporaryFile(this);
    m_tmpFileForThumbnailView->open();

    image.save(m_tmpFileForThumbnailView, "PNG");
    if (fileName == m_currentFileName)
        renderToolTip();
}

void ToolTip::requestToolTip(const DB::FileName &fileName)
{
    if (fileName.isNull() || fileName == m_currentFileName)
        return;
    m_currentFileName = fileName;
    requestImage(fileName);
}

bool ToolTip::event(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange)
        updatePalette();
    return QLabel::event(e);
}

void ToolTip::renderToolTip()
{
    const int size = Settings::SettingsData::instance()->previewSize();
    if (size != 0) {
        setText(QString::fromLatin1("<table cols=\"2\" cellpadding=\"10\"><tr><td><img src=\"%1\"></td><td>%2</td></tr>")
                    .arg(m_tmpFileForThumbnailView->fileName())
                    .arg(Utilities::createInfoText(DB::ImageDB::instance()->info(m_currentFileName), nullptr)));
    } else
        setText(QString::fromLatin1("<p>%1</p>").arg(Utilities::createInfoText(DB::ImageDB::instance()->info(m_currentFileName), nullptr)));

    setWordWrap(true);

    resize(sizeHint());
    //    m_view->setFocus();
    show();
    placeWindow();
}

} // namespace Utilities
// vi:expandtab:tabstop=4 shiftwidth=4:
