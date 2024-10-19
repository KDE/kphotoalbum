// SPDX-FileCopyrightText: 2003-2018 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "SplashScreen.h"

#include <KAboutData>
#include <KLocalizedString>
#include <QPainter>
#include <QStandardPaths>

MainWindow::SplashScreen *MainWindow::SplashScreen::s_instance = nullptr;

MainWindow::SplashScreen::SplashScreen()
    : QSplashScreen(QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QString::fromLatin1("pics/splash-large.png")))
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
    // Currently background is white, we need a contrast color
    painter->setPen(Qt::black);
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

#include "moc_SplashScreen.cpp"
