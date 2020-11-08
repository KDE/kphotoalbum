/* SPDX-FileCopyrightText: 2003-2019 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

// Qt includes
#include <QList>
#include <QWidget>

// Local includes
#include "ImagePreview.h"

#include <DB/ImageInfo.h>

class QCheckBox;
class QPushButton;
class QComboBox;
class KActionCollection;

namespace AnnotationDialog
{

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    ImagePreviewWidget(KActionCollection *actions);
    void rotate(int angle);
    void setImage(const DB::ImageInfo &info);
    void setImage(const QString &fileName);
    void setImage(const int index);
    void configure(QList<DB::ImageInfo> *imageList, bool singleEdit);
    int angle() const;
    void anticipate(DB::ImageInfo &info1);
    const QString &lastImage();
    ImagePreview *preview() const;
    bool showAreas() const;
    void canCreateAreas(bool state);
    void setFacedetectButEnabled(bool state);
    void setSearchMode(bool state);
    void updatePositionableCategories(QList<QString> positionableCategories = QList<QString>());
    QString defaultPositionableCategory() const;
    void setToggleFullscreenPreviewEnabled(bool state);

public slots:
    void slotNext();
    void slotPrev();
    void slotCopyPrevious();
    void slotDeleteImage();
    void rotateLeft();
    void rotateRight();
    void slotShowAreas(bool show);

signals:
    void imageDeleted(const DB::ImageInfo &deletedImage);
    void imageRotated(int angle);
    void imageChanged(const DB::ImageInfo &newImage);
    void indexChanged(int newIndex);
    void copyPrevClicked();
    void areaVisibilityChanged(bool visible);
    void togglePreview();

protected:
    void showEvent(QShowEvent *) override;

private: // Functions
    /**
     * Update labels and tooltip texts when canCreateAreas() changes.
     */
    void updateTexts();
    void toggleFullscreenPreview();

private: // Variables
    ImagePreview *m_preview;
    QPushButton *m_prevBut;
    QPushButton *m_nextBut;
    QPushButton *m_toggleFullscreenPreview;
    QPushButton *m_rotateLeft;
    QPushButton *m_rotateRight;
    QPushButton *m_delBut;
    QPushButton *m_copyPreviousBut;
    QPushButton *m_facedetectBut;
    QPushButton *m_toggleAreasBut;
    QList<DB::ImageInfo> *m_imageList;
    int m_current;
    bool m_singleEdit;
    QLabel *m_defaultAreaCategoryLabel;
    QComboBox *m_defaultAreaCategory;
    QWidget *m_controlWidget;
    KActionCollection *m_actions;
};
}

#endif /* IMAGEPREVIEWWIDGET_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
