// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImagePreview.h"

#include "Logging.h"
#include "ResizableFrame.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <ImageManager/AsyncLoader.h>
#include <Utilities/ImageUtil.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <QImageReader>
#include <QMouseEvent>
#include <QRubberBand>
#include <QTimer>
#include <math.h>

using namespace AnnotationDialog;

ImagePreview::ImagePreview(QWidget *parent)
    : QLabel(parent)
    , m_reloadTimer(new QTimer(this))
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(64, 64);
    // "the widget can make use of extra space, so it should get as much space as possible"
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_reloadTimer->setSingleShot(true);
    connect(m_reloadTimer, &QTimer::timeout, this, &ImagePreview::resizeFinished);
}

void ImagePreview::resizeEvent(QResizeEvent *ev)
{
    qCDebug(AnnotationDialogLog) << "Resizing from" << ev->oldSize() << "to" << ev->size();
    // during resizing, a scaled image will do
    QImage scaledImage = m_currentImage.getImage().scaled(size(), Qt::KeepAspectRatio);
    setPixmap(QPixmap::fromImage(scaledImage));
    updateScaleFactors();

    // (re)start the timer to do a full reload
    m_reloadTimer->start(200);

    QLabel::resizeEvent(ev);
}

int ImagePreview::heightForWidth(int width) const
{
    int height = width * m_aspectRatio;
    return height;
}

QSize ImagePreview::sizeHint() const
{
    QSize hint = m_info.size();
    qCDebug(AnnotationDialogLog) << "Preview size hint is" << hint;
    return hint;
}

void ImagePreview::rotate(int angle)
{
    if (!m_info.isNull()) {
        m_currentImage.setAngle(m_info.angle());
        m_info.rotate(angle, DB::RotateImageInfoOnly);
    } else {
        // Can this really happen?
        m_angle += angle;
    }

    m_preloader.cancelPreload();
    m_lastImage.reset();
    reload();

    rotateAreas(angle);
}

void ImagePreview::setImage(const DB::ImageInfo &info)
{
    m_info = info;
    reload();
}

/**
   This method should only be used for the non-user images. Currently this includes
   two images: the search image and the configure several images at a time image.
*/
void ImagePreview::setImage(const QString &fileName)
{
    m_fileName = fileName;
    m_info = DB::ImageInfo();
    m_angle = 0;
    // Set the current angle that will be passed to m_lastImage
    m_currentImage.setAngle(m_info.angle());
    reload();
}

void ImagePreview::reload()
{
    m_aspectRatio = 1;
    if (!m_info.isNull()) {
        if (m_preloader.has(m_info.fileName(), m_info.angle())) {
            qCDebug(AnnotationDialogLog) << "reload(): set preloader image";
            setCurrentImage(m_preloader.getImage());
        } else if (m_lastImage.has(m_info.fileName(), m_info.angle())) {
            qCDebug(AnnotationDialogLog) << "reload(): set last image";
            // don't pass by reference, the additional constructor is needed here
            // see setCurrentImage for the reason (where m_lastImage is changed...)
            setCurrentImage(QImage(m_lastImage.getImage()));
        } else {
            if (!m_currentImage.has(m_info.fileName(), m_info.angle())) {
                // erase old image to prevent a laggy feel,
                // but only erase old image if it is a different image
                // (otherwise we get flicker when resizing)
                setPixmap(QPixmap());
            }
            qCDebug(AnnotationDialogLog) << "reload(): set another image";
            ImageManager::AsyncLoader::instance()->stop(this);
            ImageManager::ImageRequest *request = new ImageManager::ImageRequest(m_info.fileName(), size(), m_info.angle(), this);
            request->setPriority(ImageManager::Viewer);
            ImageManager::AsyncLoader::instance()->load(request);
        }
    } else {
        qCDebug(AnnotationDialogLog) << "reload(): set image from file";
        QImage img(m_fileName);
        img = rotateAndScale(img, width(), height(), m_angle);
        setPixmap(QPixmap::fromImage(img));
    }
}

int ImagePreview::angle() const
{
    Q_ASSERT(!m_info.isNull());
    return m_angle;
}

QSize ImagePreview::getActualImageSize()
{
    if (!m_info.size().isValid()) {
        // We have to fetch the size from the image
        m_info.setSize(QImageReader(m_info.fileName().absolute()).size());
        m_aspectRatio = m_info.size().height() / m_info.size().width();
    }
    return m_info.size();
}

void ImagePreview::setCurrentImage(const QImage &image)
{
    // Cache the current image as the last image before changing it
    m_lastImage.set(m_currentImage);

    m_currentImage.set(m_info.fileName(), image, m_info.angle());
    setPixmap(QPixmap::fromImage(image));

    if (!m_anticipated.m_fileName.isNull())
        m_preloader.preloadImage(m_anticipated.m_fileName, width(), height(), m_anticipated.m_angle);

    updateScaleFactors();

    // Clear the full size image (if we have loaded one)
    m_fullSizeImage = QImage();
}

void ImagePreview::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    const DB::FileName fileName = request->databaseFileName();
    const bool loadedOK = request->loadedOK();

    if (loadedOK && !m_info.isNull()) {
        if (m_info.fileName() == fileName)
            setCurrentImage(image);
    }
}

void ImagePreview::anticipate(DB::ImageInfo &info1)
{
    // We cannot call m_preloader.preloadImage right here:
    // this function is called before reload(), so if we preload here,
    // the preloader will always be loading the image after the next image.
    m_anticipated.set(info1.fileName(), info1.angle());
}

ImagePreview::PreloadInfo::PreloadInfo()
    : m_angle(0)
{
}

void ImagePreview::PreloadInfo::set(const DB::FileName &fileName, int angle)
{
    m_fileName = fileName;
    m_angle = angle;
}

bool ImagePreview::PreviewImage::has(const DB::FileName &fileName, int angle) const
{
    return fileName == m_fileName && !m_image.isNull() && angle == m_angle;
}

QImage &ImagePreview::PreviewImage::getImage()
{
    return m_image;
}

void ImagePreview::PreviewImage::set(const DB::FileName &fileName, const QImage &image, int angle)
{
    m_fileName = fileName;
    m_image = image;
    m_angle = angle;
}

void ImagePreview::PreviewImage::set(const PreviewImage &other)
{
    m_fileName = other.m_fileName;
    m_image = other.m_image;
    m_angle = other.m_angle;
}

void ImagePreview::PreviewImage::setAngle(int angle)
{
    m_angle = angle;
}

void ImagePreview::PreviewImage::reset()
{
    m_fileName = DB::FileName();
    m_image = QImage();
}

void ImagePreview::PreviewLoader::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    if (request->loadedOK()) {
        const DB::FileName fileName = request->databaseFileName();
        set(fileName, image, request->angle());
    }
}

void ImagePreview::PreviewLoader::preloadImage(const DB::FileName &fileName, int width, int height, int angle)
{
    // no need to worry about concurrent access: everything happens in the event loop thread
    reset();
    ImageManager::AsyncLoader::instance()->stop(this);
    ImageManager::ImageRequest *request = new ImageManager::ImageRequest(fileName, QSize(width, height), angle, this);
    request->setPriority(ImageManager::ViewerPreload);
    ImageManager::AsyncLoader::instance()->load(request);
}

void ImagePreview::PreviewLoader::cancelPreload()
{
    reset();
    ImageManager::AsyncLoader::instance()->stop(this);
}

QImage ImagePreview::rotateAndScale(QImage img, int width, int height, int angle) const
{
    if (angle != 0) {
        QTransform matrix;
        matrix.rotate(angle);
        img = img.transformed(matrix);
    }
    img = Utilities::scaleImage(img, QSize(width, height), Qt::KeepAspectRatio);
    return img;
}

void ImagePreview::updateScaleFactors()
{
    if (m_info.isNull())
        return; // search mode

    // Calculate a scale factor from the original image's size and it's current preview
    QSize actualSize = getActualImageSize();
    QSize previewSize = pixmap().size();
    m_scaleWidth = double(actualSize.width()) / double(previewSize.width());
    m_scaleHeight = double(actualSize.height()) / double(previewSize.height());

    // Calculate the min and max coordinates inside the preview widget
    int previewWidth = previewSize.width();
    int previewHeight = previewSize.height();
    int widgetWidth = this->frameGeometry().width();
    int widgetHeight = this->frameGeometry().height();
    m_minX = (widgetWidth - previewWidth) / 2;
    m_maxX = m_minX + previewWidth - 1;
    m_minY = (widgetHeight - previewHeight) / 2;
    m_maxY = m_minY + previewHeight - 1;

    // Put all areas to their respective position on the preview
    remapAreas();
}

void ImagePreview::mousePressEvent(QMouseEvent *event)
{
    if (!m_areaCreationEnabled) {
        return;
    }

    if (event->button() & Qt::LeftButton) {
        if (!m_selectionRect) {
            m_selectionRect = new QRubberBand(QRubberBand::Rectangle, this);
        }

        m_areaStart = event->pos();
        if (m_areaStart.x() < m_minX || m_areaStart.x() > m_maxX || m_areaStart.y() < m_minY || m_areaStart.y() > m_maxY) {
            // Dragging started outside of the preview image
            return;
        }

        m_selectionRect->setGeometry(QRect(m_areaStart, QSize()));
        m_selectionRect->show();
    }
}

void ImagePreview::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_areaCreationEnabled) {
        return;
    }

    if (m_selectionRect && m_selectionRect->isVisible()) {
        m_currentPos = event->pos();

        // Restrict the coordinates to the preview images's size
        if (m_currentPos.x() < m_minX) {
            m_currentPos.setX(m_minX);
        }
        if (m_currentPos.y() < m_minY) {
            m_currentPos.setY(m_minY);
        }
        if (m_currentPos.x() > m_maxX) {
            m_currentPos.setX(m_maxX);
        }
        if (m_currentPos.y() > m_maxY) {
            m_currentPos.setY(m_maxY);
        }

        m_selectionRect->setGeometry(QRect(m_areaStart, m_currentPos).normalized());
    }
}

void ImagePreview::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_areaCreationEnabled) {
        return;
    }

    if (event->button() & Qt::LeftButton && m_selectionRect->isVisible()) {
        m_areaEnd = event->pos();
        processNewArea();
        m_selectionRect->hide();
    }
}

QPixmap ImagePreview::grabAreaImage(QRect area)
{
    return QPixmap::fromImage(m_currentImage.getImage().copy(area.left() - m_minX,
                                                             area.top() - m_minY,
                                                             area.width(),
                                                             area.height()));
}

QRect ImagePreview::areaPreviewToActual(QRect area) const
{
    return QRect(QPoint(int(double(area.left() - m_minX) * m_scaleWidth),
                        int(double(area.top() - m_minY) * m_scaleHeight)),
                 QPoint(int(double(area.right() - m_minX) * m_scaleWidth),
                        int(double(area.bottom() - m_minY) * m_scaleHeight)));
}

QRect ImagePreview::areaActualToPreview(QRect area) const
{
    return QRect(QPoint(int(double(area.left() / m_scaleWidth)) + m_minX,
                        int(double(area.top() / m_scaleHeight)) + m_minY),
                 QPoint(int(double(area.right() / m_scaleWidth)) + m_minX,
                        int(double(area.bottom() / m_scaleHeight)) + m_minY));
}

void ImagePreview::createNewArea(QRect geometry, QRect actualGeometry)
{
    // Create a ResizableFrame (cleaned up in Dialog::tidyAreas())
    ResizableFrame *newArea = new ResizableFrame(this);

    newArea->setGeometry(geometry);
    // Be sure not to create an invisible area
    newArea->checkGeometry();
    // In case the geometry has been changed by checkGeometry()
    actualGeometry = areaPreviewToActual(newArea->geometry());
    // Store the coordinates on the real image (not on the preview)
    newArea->setActualCoordinates(actualGeometry);
    Q_EMIT areaCreated(newArea);

    newArea->show();
    newArea->showContextMenu();
}

void ImagePreview::processNewArea()
{
    if (m_areaStart == m_areaEnd) {
        // It was just a click, no area has been dragged
        return;
    }

    QRect newAreaPreview = QRect(m_areaStart, m_currentPos).normalized();
    createNewArea(newAreaPreview, areaPreviewToActual(newAreaPreview));
}

void ImagePreview::remapAreas()
{
    const auto allAreas = this->findChildren<ResizableFrame *>();
    for (ResizableFrame *area : allAreas) {
        area->setGeometry(areaActualToPreview(area->actualCoordinates()));
    }
}

QRect ImagePreview::rotateArea(QRect originalAreaGeometry, int angle)
{
    // This is the current state of the image. We need the state before, so ...
    QSize unrotatedOriginalImageSize = getActualImageSize();
    // ... un-rotate it
    unrotatedOriginalImageSize.transpose();

    QRect rotatedAreaGeometry;
    rotatedAreaGeometry.setWidth(originalAreaGeometry.height());
    rotatedAreaGeometry.setHeight(originalAreaGeometry.width());

    if (angle == 90) {
        rotatedAreaGeometry.moveTo(
            unrotatedOriginalImageSize.height() - (originalAreaGeometry.height() + originalAreaGeometry.y()),
            originalAreaGeometry.x());
    } else {
        rotatedAreaGeometry.moveTo(
            originalAreaGeometry.y(),
            unrotatedOriginalImageSize.width() - (originalAreaGeometry.width() + originalAreaGeometry.x()));
    }

    return rotatedAreaGeometry;
}

void ImagePreview::rotateAreas(int angle)
{
    // Map all areas to their respective coordinates on the rotated actual image
    const auto allAreas = this->findChildren<ResizableFrame *>();
    for (ResizableFrame *area : allAreas) {
        area->setActualCoordinates(rotateArea(area->actualCoordinates(), angle));
    }
}

void ImagePreview::resizeFinished()
{
    qCDebug(AnnotationDialogLog) << "Reloading image after resize";
    m_preloader.cancelPreload();
    m_lastImage.reset();
    reload();
}

QRect ImagePreview::minMaxAreaPreview() const
{
    return QRect(m_minX, m_minY, m_maxX, m_maxY);
}

void ImagePreview::createTaggedArea(QString category, QString tag, QRect geometry, bool showArea)
{
    // Create a ResizableFrame (cleaned up in Dialog::tidyAreas())
    ResizableFrame *newArea = new ResizableFrame(this);

    Q_EMIT areaCreated(newArea);

    newArea->setGeometry(areaActualToPreview(geometry));
    newArea->setActualCoordinates(geometry);
    newArea->setTagData(category, tag, AutomatedChange);
    newArea->setVisible(showArea);
}

void ImagePreview::setAreaCreationEnabled(bool state)
{
    m_areaCreationEnabled = state;
}

// Currently only called when face detection/recognition is used
void ImagePreview::fetchFullSizeImage()
{
    if (m_fullSizeImage.isNull()) {
        m_fullSizeImage = QImage(m_info.fileName().absolute());
    }

    if (m_angle != m_info.angle()) {
        QTransform matrix;
        matrix.rotate(m_info.angle());
        m_fullSizeImage = m_fullSizeImage.transformed(matrix);
    }
}

void ImagePreview::acceptProposedTag(QPair<QString, QString> tagData, ResizableFrame *area)
{
    // Be sure that we do have the category the proposed tag belongs to
    bool categoryFound = false;

    // Any warnings should only happen when the recognition database is e. g. copied from another
    // database location or has been changed outside of KPA. Anyways, this m_can_ happen, so we
    // have to handle it.

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (QList<DB::CategoryPtr>::ConstIterator categoryIt = categories.constBegin();
         categoryIt != categories.constEnd(); ++categoryIt) {
        if ((*categoryIt)->name() == tagData.first) {
            if (!(*categoryIt)->positionable()) {
                KMessageBox::error(this, i18n("<p><b>Can't associate tag \"%2\"</b></p>"
                                              "<p>The category \"%1\" the tag \"%2\" belongs to is not positionable.</p>"
                                              "<p>If you want to use this tag, change this in the settings dialog. "
                                              "If this tag shouldn't be in the recognition database anymore, it can "
                                              "be deleted in the settings.</p>",
                                              tagData.first, tagData.second));
                return;
            }
            categoryFound = true;
            break;
        }
    }

    if (!categoryFound) {
        KMessageBox::error(this, i18n("<p><b>Can't associate tag \"%2\"</b></p>"
                                      "<p>The category \"%1\" the tag \"%2\" belongs to does not exist.</p>"
                                      "<p>If you want to use this tag, add this category and mark it as positionable. "
                                      "If this tag shouldn't be in the recognition database anymore, it can "
                                      "be deleted in the settings dialog.</p>",
                                      tagData.first, tagData.second));
        return;
    }

    // Tell all ListSelects that we accepted a proposed tag, so that the ListSelect
    // holding the respective category can ensure that the tag is checked
    Q_EMIT proposedTagSelected(tagData.first, tagData.second);

    // Associate the area with the proposed tag
    area->setTagData(tagData.first, tagData.second);
}

bool ImagePreview::fuzzyAreaExists(QList<QRect> &existingAreas, QRect area)
{
    float maximumDeviation;
    for (int i = 0; i < existingAreas.size(); ++i) {
        // maximumDeviation is 15% of the mean value of the width and height of each area
        maximumDeviation = float(existingAreas.at(i).width() + existingAreas.at(i).height()) * 0.075;
        if (
            distance(existingAreas.at(i).topLeft(), area.topLeft()) < maximumDeviation && distance(existingAreas.at(i).topRight(), area.topRight()) < maximumDeviation && distance(existingAreas.at(i).bottomLeft(), area.bottomLeft()) < maximumDeviation && distance(existingAreas.at(i).bottomRight(), area.bottomRight()) < maximumDeviation) {
            return true;
        }
    }

    return false;
}

float ImagePreview::distance(QPoint point1, QPoint point2)
{
    QPoint difference = point1 - point2;
    return sqrt(pow(difference.x(), 2) + pow(difference.y(), 2));
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ImagePreview.cpp"
