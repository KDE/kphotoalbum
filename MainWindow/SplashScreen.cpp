/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "SplashScreen.h"

#include <QPainter>
#include <QRegExp>
#include <QStandardPaths>

#include <KAboutData>
#include <KLocalizedString>

MainWindow::SplashScreen *MainWindow::SplashScreen::s_instance = nullptr;

MainWindow::SplashScreen::SplashScreen()
    : QSplashScreen(QStandardPaths::locate(QStandardPaths::DataLocation, QString::fromLatin1("pics/splash-large.png")))
{
    s_instance = this;
}

MainWindow::SplashScreen *MainWindow::SplashScreen::instance()
{
    return s_instance;
}

void MainWindow::SplashScreen::done()
{
    s_instance = nullptr;
    (void)close();
    deleteLater();
}

void MainWindow::SplashScreen::message(const QString &message)
{
    m_message = message;
    repaint();
}

void MainWindow::SplashScreen::drawContents(QPainter *painter)
{
    painter->save();
    QFont font = painter->font();
    font.setPointSize(10);
    painter->setFont(font);
    QRect r = QRect(QPoint(20, 265), QSize(360, 25));

    // Version String
    QString txt;
    QString version = KAboutData::applicationData().version();
    txt = i18n("%1", version);
    painter->drawText(r, Qt::AlignRight | Qt::AlignTop, txt);

    // Message
    painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, m_message);
    painter->restore();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
