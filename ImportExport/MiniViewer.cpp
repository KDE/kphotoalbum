/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "MiniViewer.h"

#include <DB/ImageInfo.h>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QTransform>

using namespace ImportExport;

MiniViewer *MiniViewer::s_instance = nullptr;

void MiniViewer::show(QImage img, DB::ImageInfoPtr info, QWidget *parent)
{
    if (!s_instance)
        s_instance = new MiniViewer(parent);

    if (info->angle() != 0) {
        QTransform matrix;
        matrix.rotate(info->angle());
        img = img.transformed(matrix);
    }
    if (img.width() > 800 || img.height() > 600)
        img = img.scaled(800, 600, Qt::KeepAspectRatio);

    s_instance->m_pixmap->setPixmap(QPixmap::fromImage(img));
    s_instance->QDialog::show();
    s_instance->raise();
}

void MiniViewer::closeEvent(QCloseEvent *)
{
    slotClose();
}

void MiniViewer::slotClose()
{
    s_instance = nullptr;
    deleteLater();
}

MiniViewer::MiniViewer(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *vlay = new QVBoxLayout(this);
    m_pixmap = new QLabel(this);
    vlay->addWidget(m_pixmap);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close, this);
    box->button(QDialogButtonBox::Close)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(box, &QDialogButtonBox::rejected, this, &MiniViewer::slotClose);
    vlay->addWidget(box);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
