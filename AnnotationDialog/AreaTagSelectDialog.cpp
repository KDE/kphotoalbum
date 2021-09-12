/* SPDX-FileCopyrightText: 2016-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "AreaTagSelectDialog.h"

#include "CompletableLineEdit.h"
#include "Dialog.h"
#include "ListSelect.h"
#include "ResizableFrame.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>

AnnotationDialog::AreaTagSelectDialog::AreaTagSelectDialog(AnnotationDialog::ResizableFrame *area,
                                                           ListSelect *ls,
                                                           QPixmap areaImage,
                                                           Dialog *dialog)
    : QDialog(area)
    , m_area(area)
    , m_listSelect(ls)
    , m_dialog(dialog)
    , m_usedTags(dialog->positionedTags(ls->category()))
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QGridLayout *mainLayout = new QGridLayout(this);

    m_areaImageLabel = new QLabel();
    m_areaImageLabel->setAlignment(Qt::AlignTop);
    m_areaImageLabel->setPixmap(areaImage);
    // span 2 rows:
    mainLayout->addWidget(m_areaImageLabel, 0, 0, 2, 1);

    CompletableLineEdit *tagSelect = new CompletableLineEdit(ls, this);
    ls->connectLineEdit(tagSelect);
    tagSelect->setAlignment(Qt::AlignTop);
    mainLayout->addWidget(tagSelect, 0, 1);

    m_messageLabel = new QLabel();
    m_messageLabel->setWordWrap(true);
    mainLayout->addWidget(m_messageLabel, 1, 1);
    m_messageLabel->hide();

    QMenu *tagMenu = new QMenu();
    m_area->addTagActions(tagMenu);
    mainLayout->addWidget(tagMenu, 2, 0, 1, 2);
    connect(tagMenu, &QMenu::triggered, this, &QDialog::accept);

    connect(tagSelect, &KLineEdit::returnPressed, this, &AreaTagSelectDialog::slotSetTag);
    connect(tagSelect, &QLineEdit::textChanged, this, &AreaTagSelectDialog::slotValidateTag);
    connect(this, &QDialog::finished, this, &AreaTagSelectDialog::slotFinished);
}

void AnnotationDialog::AreaTagSelectDialog::slotSetTag(const QString &tag)
{
    QString enteredText = tag.trimmed();
    if (m_dialog->positionableTagAvailable(m_listSelect->category(), enteredText)) {
        const auto currentTagData = m_area->tagData();
        // was there already a tag associated?
        if (!currentTagData.first.isEmpty()) {
            // Deselect the tag
            m_dialog->listSelectForCategory(currentTagData.first)->deselectTag(currentTagData.second);
            m_area->removeTagData();
        }
        m_area->setTagData(m_listSelect->category(), enteredText);
        this->accept();
    }
}

void AnnotationDialog::AreaTagSelectDialog::slotValidateTag(const QString &tag)
{
    if (m_usedTags.contains(tag.trimmed())) {
        m_messageLabel->show();
        m_messageLabel->setText(i18n("Tag is already used for another area"));
        adjustSize();
    } else {
        m_messageLabel->clear();
        adjustSize();
    }
}

void AnnotationDialog::AreaTagSelectDialog::slotFinished()
{
    // remove filter from listSelect
    m_listSelect->showOnlyItemsMatching(QString());
}

void AnnotationDialog::AreaTagSelectDialog::paintEvent(QPaintEvent *)
{
    QColor backgroundColor = palette().base().color();
    backgroundColor.setAlpha(160);
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor);
}

void AnnotationDialog::AreaTagSelectDialog::moveToArea(QPoint areaTopLeft)
{
    move(areaTopLeft - (m_areaImageLabel->mapToGlobal(QPoint(0, 0)) - mapToGlobal(QPoint(0, 0))));
}

// vi:expandtab:tabstop=4 shiftwidth=4:
