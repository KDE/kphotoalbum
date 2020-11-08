/* SPDX-FileCopyrightText: 2003-2018 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageCounter.h"

#include <KLocalizedString>
#include <QLabel>

MainWindow::ImageCounter::ImageCounter(QWidget *parent)
    : QLabel(parent)
{
    setText(QString::fromLatin1("---"));
    setMargin(5);
}

void MainWindow::ImageCounter::setMatchCount(uint matches)
{
    setText(i18np("Showing 1 thumbnail", "Showing %1 thumbnails", matches));
}

void MainWindow::ImageCounter::setSelectionCount(uint selected)
{
    if (selected > 0)
        setText(i18n("(%1 selected)", selected));
    else
        setText(QString());
}

void MainWindow::ImageCounter::setTotal(uint c)
{
    setText(i18n("Total: %1", c));
}

void MainWindow::ImageCounter::showBrowserMatches(uint matches)
{
    setText(i18np("1 match", "%1 matches", matches));
}

// vi:expandtab:tabstop=4 shiftwidth=4:
