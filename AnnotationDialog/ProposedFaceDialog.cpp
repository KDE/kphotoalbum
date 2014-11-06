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

// Qt includes
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPoint>
#include <QTimer>
#include <QToolButton>
#include <QCursor>

// KDE includes
#include <KLocale>
#include <KIconLoader>

// Local includes
#include "ProposedFaceDialog.h"

AnnotationDialog::ProposedFaceDialog::ProposedFaceDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    setMouseTracking(true);

    m_area = dynamic_cast<ResizableFrame *>(parent);

    QHBoxLayout *layout = new QHBoxLayout(this);

    QToolButton *acceptButton = new QToolButton;
    acceptButton->setIcon(KIcon(QString::fromUtf8("dialog-ok-apply")));
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(acceptTag()));
    layout->addWidget(acceptButton);

    QLabel *label = new QLabel(i18n("Is this %1 (%2)?",
                                    m_area->proposedTagData().second,
                                    m_area->proposedTagData().first));
    layout->addWidget(label);

    QToolButton *declineButton = new QToolButton;
    declineButton->setIcon(KIcon(QString::fromUtf8("application-exit")));
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

#ifdef HAVE_KFACE
void AnnotationDialog::ProposedFaceDialog::leaveEvent(QEvent *)
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
#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
