// SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2009-2010 Hassan Ibraheem <hasan.ibraheem@gmail.com>
// SPDX-FileCopyrightText: 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2010-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2019 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImagePreviewWidget.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <MainWindow/DeleteDialog.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

using namespace AnnotationDialog;

ImagePreviewWidget::ImagePreviewWidget(KActionCollection *actions, const QList<DB::ImageInfo> *imageList)
    : QWidget()
    , m_singleEdit(false)
    , m_actions(actions)
    , m_imageList(imageList)
{
    Q_ASSERT(actions);
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_preview = new ImagePreview(this);
    layout->addWidget(m_preview, 1);
    connect(this, &ImagePreviewWidget::areaVisibilityChanged, m_preview, &ImagePreview::setAreaCreationEnabled);

    m_controlWidget = new QWidget;
    layout->addWidget(m_controlWidget);
    QVBoxLayout *controlLayout = new QVBoxLayout(m_controlWidget);
    QHBoxLayout *controlButtonsLayout = new QHBoxLayout;
    controlLayout->addLayout(controlButtonsLayout);
    controlButtonsLayout->addStretch(1);

    m_prevBut = new QPushButton(this);
    m_prevBut->setIcon(QIcon::fromTheme(QString::fromLatin1("arrow-left")));
    m_prevBut->setFixedWidth(40);
    controlButtonsLayout->addWidget(m_prevBut);
    m_prevBut->setToolTip(i18n("Annotate previous image (Page Up)"));

    m_nextBut = new QPushButton(this);
    m_nextBut->setIcon(QIcon::fromTheme(QString::fromLatin1("arrow-right")));
    m_nextBut->setFixedWidth(40);
    controlButtonsLayout->addWidget(m_nextBut);
    m_nextBut->setToolTip(i18n("Annotate next image (Page Down)"));

    controlButtonsLayout->addStretch(1);

    m_toggleFullscreenPreview = new QPushButton;
    m_toggleFullscreenPreview->setIcon(QIcon::fromTheme(QString::fromUtf8("file-zoom-in")));
    m_toggleFullscreenPreview->setFixedWidth(40);

    controlButtonsLayout->addWidget(m_toggleFullscreenPreview);
    connect(m_toggleFullscreenPreview, &QPushButton::clicked, this, &ImagePreviewWidget::toggleFullscreenPreview);

    m_rotateLeft = new QPushButton(this);
    controlButtonsLayout->addWidget(m_rotateLeft);
    m_rotateLeft->setIcon(QIcon::fromTheme(QString::fromLatin1("object-rotate-left")));
    m_rotateLeft->setFixedWidth(40);

    m_rotateRight = new QPushButton(this);
    controlButtonsLayout->addWidget(m_rotateRight);
    m_rotateRight->setIcon(QIcon::fromTheme(QString::fromLatin1("object-rotate-right")));
    m_rotateRight->setFixedWidth(40);

    m_copyPreviousBut = new QPushButton(this);
    controlButtonsLayout->addWidget(m_copyPreviousBut);
    m_copyPreviousBut->setIcon(QIcon::fromTheme(QString::fromLatin1("go-bottom")));
    m_copyPreviousBut->setFixedWidth(40);
    m_copyPreviousBut->setWhatsThis(i18nc("@info:whatsthis", "<para>Set the same tags on this image than on the previous one. The image date, label, rating, and description are left unchanged.</para>"));

    m_toggleAreasBut = new QPushButton(this);
    controlButtonsLayout->addWidget(m_toggleAreasBut);
    m_toggleAreasBut->setIcon(QIcon::fromTheme(QString::fromLatin1("document-preview")));
    m_toggleAreasBut->setFixedWidth(40);
    m_toggleAreasBut->setCheckable(true);
    m_toggleAreasBut->setChecked(true);
    // tooltip text is set in updateTexts()

    controlButtonsLayout->addStretch(1);
    m_delBut = new QPushButton(this);
    m_delBut->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-delete")));
    controlButtonsLayout->addWidget(m_delBut);
    m_delBut->setAutoDefault(false);

    controlButtonsLayout->addStretch(1);

    connect(m_copyPreviousBut, &QPushButton::clicked, this, &ImagePreviewWidget::slotCopyPrevious);
    connect(m_delBut, &QPushButton::clicked, this, &ImagePreviewWidget::slotDeleteImage);
    connect(m_nextBut, &QPushButton::clicked, this, &ImagePreviewWidget::slotNext);
    connect(m_prevBut, &QPushButton::clicked, this, &ImagePreviewWidget::slotPrev);
    connect(m_rotateLeft, &QPushButton::clicked, this, &ImagePreviewWidget::rotateLeft);
    connect(m_rotateRight, &QPushButton::clicked, this, &ImagePreviewWidget::rotateRight);
    connect(m_toggleAreasBut, &QPushButton::clicked, this, &ImagePreviewWidget::slotShowAreas);

    QHBoxLayout *defaultAreaCategoryLayout = new QHBoxLayout;
    controlLayout->addLayout(defaultAreaCategoryLayout);
    m_defaultAreaCategoryLabel = new QLabel(i18n("Category for new areas:"));
    m_defaultAreaCategoryLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    defaultAreaCategoryLayout->addWidget(m_defaultAreaCategoryLabel);
    m_defaultAreaCategory = new QComboBox(this);
    defaultAreaCategoryLayout->addWidget(m_defaultAreaCategory);

    m_current = -1;
    updateTexts();
}

void ImagePreviewWidget::updatePositionableCategories(QList<QString> positionableCategories)
{
    if (positionableCategories.size() <= 1) {
        m_defaultAreaCategoryLabel->hide();
        m_defaultAreaCategory->hide();
    } else {
        m_defaultAreaCategoryLabel->show();
        m_defaultAreaCategory->show();
    }

    m_defaultAreaCategory->clear();

    for (const QString &categoryName : positionableCategories) {
        m_defaultAreaCategory->addItem(categoryName);
    }
}

QString ImagePreviewWidget::defaultPositionableCategory() const
{
    return m_defaultAreaCategory->currentText();
}

int ImagePreviewWidget::angle() const
{
    return m_preview->angle();
}

void ImagePreviewWidget::anticipate(DB::ImageInfo &info1)
{
    m_preview->anticipate(info1);
}

void ImagePreviewWidget::configure(bool singleEdit)
{
    m_current = 0;
    setImage(m_imageList->at(m_current));
    m_singleEdit = singleEdit;

    m_delBut->setEnabled(m_singleEdit);
    m_copyPreviousBut->setEnabled(m_singleEdit);
    m_rotateLeft->setEnabled(m_singleEdit);
    m_rotateRight->setEnabled(m_singleEdit);
}

void ImagePreviewWidget::updateAfterDiscard(int index, const DB::FileNameList &fileNames)
{
    if (index != -1) {
        // AnnotationDialog::Dialog::slotDiscardFiles could determine the image to show
        m_current = index;
    } else {
        // If multiple images are annotated at once, index is always -1.
        // Try to find the currently showed image in this case, or fall back on another one.
        const auto index = fileNames.indexOf(m_preview->currentInfo().fileName());
        if (index != -1) {
            m_current = index;
        } else {
            if (m_current == m_imageList->count()) {
                m_current = m_imageList->count() - 1;
            }
        }
    }
    setImage(m_imageList->at(m_current));
}

void ImagePreviewWidget::slotPrev()
{
    if ((m_current <= 0))
        return;

    m_current--;
    if (m_current != 0)
        m_preview->anticipate((*m_imageList)[m_current - 1]);
    setImage(m_imageList->at(m_current));

    Q_EMIT indexChanged(m_current);
}

void ImagePreviewWidget::slotNext()
{
    if ((m_current == -1) || (m_current == (int)m_imageList->count() - 1))
        return;

    m_current++;

    if (m_current != (int)m_imageList->count() - 1)
        m_preview->anticipate((*m_imageList)[m_current + 1]);
    setImage(m_imageList->at(m_current));

    Q_EMIT indexChanged(m_current);
}

void ImagePreviewWidget::slotCopyPrevious()
{
    Q_EMIT copyPrevClicked();
}

void ImagePreviewWidget::rotateLeft()
{
    rotate(-90);
}

void ImagePreviewWidget::rotateRight()
{
    rotate(90);
}

void ImagePreviewWidget::rotate(int angle)
{
    if (!m_singleEdit)
        return;

    m_preview->rotate(angle);

    Q_EMIT imageRotated(angle);
}

void ImagePreviewWidget::slotDeleteImage()
{
    if (!m_singleEdit)
        return;

    MainWindow::DeleteDialog dialog(this);
    DB::ImageInfo info = m_imageList->at(m_current);

    const DB::FileNameList deleteList = DB::FileNameList() << info.fileName();

    int ret = dialog.exec(deleteList);
    if (ret == QDialog::Rejected) // Delete Dialog rejected, do nothing
        return;
}

void ImagePreviewWidget::setImage(const DB::ImageInfo &info)
{
    m_nextBut->setEnabled(m_current != (int)m_imageList->count() - 1);
    m_prevBut->setEnabled(m_current != 0);
    m_copyPreviousBut->setEnabled(m_current != 0 && m_singleEdit);

    m_preview->setImage(info);

    Q_EMIT imageChanged(info);
}

void ImagePreviewWidget::setImage(const int index)
{
    m_current = index;

    setImage(m_imageList->at(m_current));
}

void ImagePreviewWidget::setImage(const QString &fileName)
{
    m_preview->setImage(fileName);
    m_current = -1;

    m_nextBut->setEnabled(false);
    m_prevBut->setEnabled(false);
    m_rotateLeft->setEnabled(false);
    m_rotateRight->setEnabled(false);
    m_delBut->setEnabled(false);
    m_copyPreviousBut->setEnabled(false);
}

ImagePreview *ImagePreviewWidget::preview() const
{
    return m_preview;
}

void ImagePreviewWidget::slotShowAreas(bool show)
{
    // slot can be triggered by something else than the button:
    m_toggleAreasBut->setChecked(show);

    Q_EMIT areaVisibilityChanged(show);
}

bool ImagePreviewWidget::showAreas() const
{
    return m_toggleAreasBut->isChecked();
}

void ImagePreviewWidget::canCreateAreas(bool state)
{
    if (m_toggleAreasBut->isEnabled() != state) {
        m_toggleAreasBut->setChecked(state);
        m_toggleAreasBut->setEnabled(state);
        Q_EMIT areaVisibilityChanged(state);
    }
    m_preview->setAreaCreationEnabled(state);
    updateTexts();
}

void ImagePreviewWidget::updateTexts()
{
    if (m_toggleAreasBut->isEnabled()) {
        // positionable tags enabled
        m_toggleAreasBut->setToolTip(i18nc("@info:tooltip", "Hide or show areas on the image"));
    } else {
        if (m_singleEdit) {
            // positionable tags disabled
            m_toggleAreasBut->setToolTip(i18nc("@info:tooltip",
                                               "If you enable <emphasis>positionable tags</emphasis> for at least one category in "
                                               "<interface>Settings|Configure KPhotoAlbum...|Categories</interface>, you can "
                                               "associate specific image areas with tags."));
        } else {
            m_toggleAreasBut->setToolTip(i18nc("@info:tooltip",
                                               "Areas on an image can only be shown in single-image annotation mode."));
        }
    }
}

void ImagePreviewWidget::setSearchMode(bool state)
{
    m_controlWidget->setVisible(!state);
}

void ImagePreviewWidget::toggleFullscreenPreview()
{
    Q_EMIT togglePreview();
}

void ImagePreviewWidget::setToggleFullscreenPreviewEnabled(bool state)
{
    m_toggleFullscreenPreview->setEnabled(state);
}

void AnnotationDialog::ImagePreviewWidget::showEvent(QShowEvent *)
{
    auto setToolTip = [this](QWidget *widget, QString tooltip, const char *key) {
        const auto action = m_actions->action(QString::fromLatin1(key));
        QKeySequence shortcut = action ? action->shortcut() : QKeySequence();
        if (!shortcut.isEmpty())
            tooltip += QString::fromLatin1(" (%1)").arg(shortcut.toString());
        widget->setToolTip(tooltip);
    };

    setToolTip(m_toggleFullscreenPreview, i18nc("@info:tooltip", "Toggle full-screen preview"), "annotationdialog-toggle-viewer");
    setToolTip(m_rotateLeft, i18nc("@info:tooltip", "Rotate counterclockwise"), "annotationdialog-rotate-left");
    setToolTip(m_rotateRight, i18nc("@info:tooltip", "Rotate clockwise"), "annotationdialog-rotate-right");
    setToolTip(m_copyPreviousBut, i18nc("@info:tooltip", "Copy tags from previously tagged image"), "annotationdialog-copy-previous");
    setToolTip(m_delBut, i18nc("@info:tooltip", "Delete image"), "annotationdialog-delete-image");
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ImagePreviewWidget.cpp"
