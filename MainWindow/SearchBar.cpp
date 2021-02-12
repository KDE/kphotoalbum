// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "SearchBar.h"

#include <KLocalizedString>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <kactioncollection.h>
#include <kmainwindow.h>
#include <qapplication.h>
#include <qlabel.h>

MainWindow::SearchBar::SearchBar(KMainWindow *parent)
    : KToolBar(parent)
{
    setWindowTitle(i18nc("Name/title of the search bar toolbar widget", "Search Bar"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("search")));

    m_edit = new QLineEdit(this);
    m_edit->setClearButtonEnabled(true);
    m_edit->setPlaceholderText(i18nc("@label:textbox", "Search…"));

    addWidget(m_edit);
    connect(m_edit, &QLineEdit::textChanged, this, &SearchBar::textChanged);
    connect(m_edit, &QLineEdit::returnPressed, this, &SearchBar::returnPressed);

    m_edit->installEventFilter(this);
}

bool MainWindow::SearchBar::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right || ke->key() == Qt::Key_PageDown || ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_Home || ke->key() == Qt::Key_End) {
            emit movementKeyPressed(ke);
            return true;
        } else if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return) {
            // If I don't interpret return and enter here, but simply rely
            // on QLineEdit itself to emit the signal, then it will
            // propagate to the main window, and from there be delivered to
            // the central widget.
            emit returnPressed();
            return true;
        } else if (ke->key() == Qt::Key_Escape)
            clear();
    }
    return false;
}

void MainWindow::SearchBar::clear()
{
    m_edit->clear();
    emit cleared();
}

/**
 * This was originally just a call to setEnabled() on the SearchBar itself,
 * but due to a bug in either KDE or Qt, this resulted in the bar never
 * being enabled again after a disable.
 */
void MainWindow::SearchBar::setLineEditEnabled(bool enabled)
{
    m_edit->setEnabled(enabled);
    m_edit->setFocus();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
