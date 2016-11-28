/* Copyright (C) 2016 Tobias Leupold <tobias.leupold@web.de>

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
#include <QApplication>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QSet>

AnnotationDialog::AreaTagSelectDialog::AreaTagSelectDialog(AnnotationDialog::ResizableFrame *area, ListSelect *ls, QPixmap &areaImage, Dialog *dialog)
    :QDialog(area)
    , m_area(area)
    , m_dialog(dialog)
    , m_usedTags(dialog->positionableTags(ls->category()))
    , m_category(ls->category())
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QLabel* areaImageLabel = new QLabel();
    areaImageLabel->setPixmap(areaImage);
    mainLayout->addWidget(areaImageLabel);

    QVBoxLayout *vLayout = new QVBoxLayout;
    mainLayout->addLayout(vLayout);

    CompletableLineEdit* tagSelect = new CompletableLineEdit(ls, this);
    ls->connectLineEdit(tagSelect);
    vLayout->addWidget(tagSelect);

    m_messageLabel = new QLabel();
    vLayout->addWidget(m_messageLabel);

    connect(tagSelect, &KLineEdit::returnPressed, this, &AreaTagSelectDialog::slotSetTag);
    connect(tagSelect, &QLineEdit::textChanged, this, &AreaTagSelectDialog::slotValidateTag);
}

void AnnotationDialog::AreaTagSelectDialog::slotSetTag(const QString &tag)
{
    QString enteredText = tag.trimmed();
    if (m_dialog->positionableTagValid(m_category, enteredText))
    {
        m_area->setTagData(m_category, enteredText);
        this->accept();
    }
}

void AnnotationDialog::AreaTagSelectDialog::slotValidateTag(const QString &tag)
{
    QString enteredText = tag.trimmed();

    if(m_usedTags.contains(enteredText))
    {
        m_messageLabel->setText(
                    i18n("Tag already used for another area")
                    );
    } else {
        m_messageLabel->clear();
    }
}

void AnnotationDialog::AreaTagSelectDialog::paintEvent(QPaintEvent*)
{
    QColor backgroundColor = Qt::white;
    backgroundColor.setAlpha(160);
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
