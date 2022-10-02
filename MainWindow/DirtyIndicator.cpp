// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DirtyIndicator.h"

#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <kiconloader.h>

static MainWindow::DirtyIndicator *s_instance = nullptr;
bool MainWindow::DirtyIndicator::s_autoSaveDirty = false;
bool MainWindow::DirtyIndicator::s_saveDirty = false;
bool MainWindow::DirtyIndicator::s_suppressMarkDirty = false;

MainWindow::DirtyIndicator::DirtyIndicator(QWidget *parent)
    : QLabel(parent)
{
    m_dirtyPix = QIcon::fromTheme(QString::fromLatin1("media-floppy"))
                     .pixmap(KIconLoader::StdSizes::SizeSmall);
    setFixedWidth(m_dirtyPix.width() + 10);
    s_instance = this;

    // Might have been marked dirty even before the indicator had been created, by the database searching during loading.
    if (s_saveDirty)
        markDirty();
}

void MainWindow::DirtyIndicator::suppressMarkDirty(bool state)
{
    MainWindow::DirtyIndicator::s_suppressMarkDirty = state;
}

void MainWindow::DirtyIndicator::markDirty()
{
    if (MainWindow::DirtyIndicator::s_suppressMarkDirty) {
        return;
    }

    if (s_instance) {
        s_instance->markDirtySlot();
    } else {
        s_saveDirty = true;
        s_autoSaveDirty = true;
    }
}

void MainWindow::DirtyIndicator::markDirtySlot()
{
    if (MainWindow::DirtyIndicator::s_suppressMarkDirty) {
        return;
    }

    s_saveDirty = true;
    s_autoSaveDirty = true;
    setPixmap(m_dirtyPix);
    Q_EMIT dirty();
}

void MainWindow::DirtyIndicator::autoSaved()
{
    s_autoSaveDirty = false;
}

void MainWindow::DirtyIndicator::saved()
{
    s_autoSaveDirty = false;
    s_saveDirty = false;
    setPixmap(QPixmap());
}

bool MainWindow::DirtyIndicator::isSaveDirty() const
{
    return s_saveDirty;
}

bool MainWindow::DirtyIndicator::isAutoSaveDirty() const
{
    return s_autoSaveDirty;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DirtyIndicator.cpp"
