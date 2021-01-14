/* SPDX-FileCopyrightText: 2014-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ConnectionIndicator.h"

#include "RemoteInterface.h"

#include <MainWindow/Options.h>
#include <kpabase/SettingsData.h>

#include <KLocalizedString>
#include <QDialog>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QValidator>

namespace RemoteControl
{

ConnectionIndicator::ConnectionIndicator(QWidget *parent)
    : QLabel(parent)
    , m_state(Off)
{
    setToolTip(i18n("This icon indicates if KPhotoAlbum is connected to an android device.\n"
                    "Click on the icon to toggle listening for clients in the local area network.\n"
                    "If the local area network doesn't allow broadcast packages between the android client "
                    "and KPhotoAlbum, then right click on the icon and specify the android device's address.\n"
                    "The android client can be downloaded from google play."));

    connect(&RemoteInterface::instance(), SIGNAL(connected()), this, SLOT(on()));
    connect(&RemoteInterface::instance(), SIGNAL(disConnected()), this, SLOT(wait()));
    connect(&RemoteInterface::instance(), SIGNAL(listening()), this, SLOT(wait()));
    connect(&RemoteInterface::instance(), SIGNAL(stoppedListening()), this, SLOT(off()));

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ConnectionIndicator::waitingAnimation);

    off();
}

void ConnectionIndicator::mouseReleaseEvent(QMouseEvent *)
{
    if (m_state == Off) {
        QHostAddress bindTo = MainWindow::Options::the()->listen();
        if (bindTo.isNull())
            bindTo = QHostAddress::Any;
        RemoteInterface::instance().listen(bindTo);

        wait();
    } else {
        RemoteInterface::instance().stopListening();
        m_state = Off;
        m_timer->stop();
        off();
    }
}

namespace
{
    class IPValidator : public QValidator
    {
    protected:
        State validate(QString &input, int &) const override
        {
            for (int pos = 0; pos < 15; pos += 4) {
                bool ok1;
                int i = input.mid(pos, 1).toInt(&ok1);
                bool ok2;
                int j = input.mid(pos + 1, 1).toInt(&ok2);
                bool ok3;
                int k = input.mid(pos + 2, 1).toInt(&ok3);

                if ((ok1 && i > 2) || (ok1 && ok2 && i == 2 && j > 5) || (ok1 && ok2 && ok3 && i * 100 + j * 10 + k > 255))
                    return Invalid;
            }
            return Acceptable;
        }
    };
} //namespace

void ConnectionIndicator::contextMenuEvent(QContextMenuEvent *)
{
    QDialog dialog;
    QLabel label(i18n("Android device address: "), &dialog);
    QLineEdit edit(&dialog);
    edit.setInputMask(QString::fromUtf8("000.000.000.000;_"));
    edit.setText(Settings::SettingsData::instance()->recentAndroidAddress());
    IPValidator validator;
    edit.setValidator(&validator);

    QHBoxLayout layout(&dialog);
    layout.addWidget(&label);
    layout.addWidget(&edit);

    connect(&edit, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    int code = dialog.exec();

    if (code == QDialog::Accepted) {
        RemoteInterface::instance().connectTo(QHostAddress(edit.text()));
        wait();
        Settings::SettingsData::instance()->setRecentAndroidAddress(edit.text());
    }
}

void ConnectionIndicator::on()
{
    m_state = On;
    m_timer->stop();
    QIcon icon { QIcon::fromTheme(QString::fromUtf8("network-wireless")) };
    setPixmap(icon.pixmap(32, 32));
}

void ConnectionIndicator::off()
{
    m_timer->stop();
    m_state = Off;
    QIcon icon { QIcon::fromTheme(QString::fromUtf8("network-disconnect")) };
    setPixmap(icon.pixmap(32, 32));
}

void ConnectionIndicator::wait()
{
    m_timer->start(300);
    m_state = Connecting;
}

void ConnectionIndicator::waitingAnimation()
{
    static int index = 0;
    static QList<QPixmap> icons;
    if (icons.isEmpty()) {
        icons.append(QIcon::fromTheme(QString::fromUtf8("network-wireless-disconnected")).pixmap(32, 32));
        icons.append(QIcon::fromTheme(QString::fromUtf8("network-wireless-connected-25")).pixmap(32, 32));
        icons.append(QIcon::fromTheme(QString::fromUtf8("network-wireless-connected-50")).pixmap(32, 32));
        icons.append(QIcon::fromTheme(QString::fromUtf8("network-wireless-connected-75")).pixmap(32, 32));
        icons.append(QIcon::fromTheme(QString::fromUtf8("network-wireless")).pixmap(32, 32));
    }
    index = (index + 1) % icons.count();
    setPixmap(icons[index]);
}

} // namespace RemoteControl
