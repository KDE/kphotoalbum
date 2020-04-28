/* Copyright (C) 2016-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
