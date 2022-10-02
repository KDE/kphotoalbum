// SPDX-FileCopyrightText: 2003-2019 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
    void setSearchMode(bool state);
    void updatePositionableCategories(QList<QString> positionableCategories = QList<QString>());
    QString defaultPositionableCategory() const;
    void setToggleFullscreenPreviewEnabled(bool state);

public Q_SLOTS:
    void slotNext();
    void slotPrev();
    void slotCopyPrevious();
    void slotDeleteImage();
    void rotateLeft();
    void rotateRight();
    void slotShowAreas(bool show);

Q_SIGNALS:
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
    ImagePreview *m_preview = nullptr;
    QPushButton *m_prevBut = nullptr;
    QPushButton *m_nextBut = nullptr;
    QPushButton *m_toggleFullscreenPreview = nullptr;
    QPushButton *m_rotateLeft = nullptr;
    QPushButton *m_rotateRight = nullptr;
    QPushButton *m_delBut = nullptr;
    QPushButton *m_copyPreviousBut = nullptr;
    QPushButton *m_toggleAreasBut = nullptr;
    QList<DB::ImageInfo> *m_imageList = nullptr;
    int m_current = -1;
    bool m_singleEdit = false;
    QLabel *m_defaultAreaCategoryLabel = nullptr;
    QComboBox *m_defaultAreaCategory = nullptr;
    QWidget *m_controlWidget = nullptr;
    KActionCollection *m_actions = nullptr;
};
}

#endif /* IMAGEPREVIEWWIDGET_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
