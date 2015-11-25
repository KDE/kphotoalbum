/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ProposedFaceDialog.h"

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <QToolButton>
#include <QCursor>
#include <QPainter>

// KDE includes
#include <KLocale>
#include <KIconLoader>


AnnotationDialog::ProposedFaceDialog::ProposedFaceDialog(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);

    m_area = dynamic_cast<ResizableFrame*>(parent);

    QHBoxLayout* layout = new QHBoxLayout(this);

    QString buttonStyle = QString::fromUtf8(
            "QToolButton { margin: 0px; padding: 4px; border-radius: 6px; background-color: rgba( 255, 255, 255, 200); }"
            "QToolButton:pressed { background-color: rgba( 255, 255, 255, 100); }"
            "QToolButton:hover { border: 1px solid #ccc; }"
            );

    QToolButton* acceptButton = new QToolButton;
    acceptButton->setIcon(KIcon(QString::fromUtf8("dialog-ok-apply")));
    acceptButton->setStyleSheet( buttonStyle );
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(acceptTag()));
    layout->addWidget(acceptButton);

    QLabel* isThisLabel = new QLabel(i18n("Is this %1 (%2)?",
                                          m_area->proposedTagData().second,
                                          m_area->proposedTagData().first));
    layout->addWidget(isThisLabel);

    QToolButton* declineButton = new QToolButton;
    declineButton->setIcon(KIcon(QString::fromUtf8("dialog-close")));
    declineButton->setStyleSheet( buttonStyle );
    connect(declineButton, SIGNAL(clicked()), this, SLOT(declineTag()));
    layout->addWidget(declineButton);

    QPoint pos = QCursor::pos();
    pos.setX(pos.x() - 20);
    move(pos);
    show();
}

AnnotationDialog::ProposedFaceDialog::~ProposedFaceDialog()
{
}

void AnnotationDialog::ProposedFaceDialog::paintEvent(QPaintEvent*)
{
    QColor backgroundColor = Qt::white;
    backgroundColor.setAlpha(160);
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor);
}

void AnnotationDialog::ProposedFaceDialog::leaveEvent(QEvent*)
{
    QTimer::singleShot(0, m_area, SLOT(checkUnderMouse()));
}

void AnnotationDialog::ProposedFaceDialog::removeMe()
{
    deleteLater();
    m_area->proposedFaceDialogRemoved();
}

void AnnotationDialog::ProposedFaceDialog::checkUnderMouse()
{
    if (! underMouse()) {
        removeMe();
    }
}

void AnnotationDialog::ProposedFaceDialog::acceptTag()
{
    m_area->acceptTag();
    removeMe();
}

void AnnotationDialog::ProposedFaceDialog::declineTag()
{
    m_area->removeProposedTagData();
    removeMe();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
