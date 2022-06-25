/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "MiniViewer.h"

#include <DB/ImageInfo.h>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
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

#include "moc_MiniViewer.cpp"
