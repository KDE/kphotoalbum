// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H
#include <QSplashScreen>

namespace MainWindow
{

class SplashScreen : public QSplashScreen
{
    Q_OBJECT

public:
    SplashScreen();
    static SplashScreen *instance();
    void done();
    void message(const QString &message);

protected:
    void drawContents(QPainter *painter) override;

private:
    static SplashScreen *s_instance;
    QString m_message;
};
}

#endif /* SPLASHSCREEN_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
