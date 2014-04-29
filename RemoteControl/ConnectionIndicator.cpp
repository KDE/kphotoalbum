#include "ConnectionIndicator.h"
#include <KIcon>
#include "RemoteInterface.h"
#include <QDialog>
#include <QTimer>
#include <QLabel>
#include <KLocale>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QHostAddress>

namespace RemoteControl {

ConnectionIndicator::ConnectionIndicator(QWidget* parent) :
    QLabel(parent), m_state(Off)
{
    setToolTip(i18n("This icon indicates if KPhotoAlbum is connected to an android device.\n"
                    "Click on the icon to toggle listening for clients in the local area network.\n"
                    "If the local area network doesn't allow broadcast packages between the android client "
                    "and KPhotoAlbum, then right click on the icon and specify the android device's address.\n"
                    "The android client can be downloaded from google play."));

    connect(&RemoteInterface::instance(), SIGNAL(connected()), this, SLOT(on()));
    connect(&RemoteInterface::instance(), SIGNAL(disConnected()), this, SLOT(off()));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(waitingAnimation()));

    off();
}

void ConnectionIndicator::mouseReleaseEvent(QMouseEvent*)
{
    if (m_state == Off) {
        m_timer->start(300);
        m_state = Connecting;
        RemoteInterface::instance().listen();
    }
    else {
        RemoteInterface::instance().stopListening();
        m_state = Off;
        m_timer->stop();
        off();
    }
}

void ConnectionIndicator::contextMenuEvent(QContextMenuEvent*)
{
    QDialog dialog;
    QLabel label(i18n("Android device address: "), &dialog);
    QLineEdit edit(&dialog);
    edit.setText(QString::fromUtf8("192.168.42.129"));

    QHBoxLayout layout(&dialog);
    layout.addWidget(&label);
    layout.addWidget(&edit);

    connect(&edit, SIGNAL(returnPressed()), &dialog, SLOT(accept()));
    int code = dialog.exec();

    if (code == QDialog::Accepted) {
        RemoteInterface::instance().connectTo(QHostAddress(edit.text()));
        m_timer->start(300);
        m_state = Connecting;
    }
}

void ConnectionIndicator::on()
{
    m_state = On;
    m_timer->stop();
    setPixmap(KIcon(QString::fromUtf8("network-wireless")).pixmap(32,32));
}

void ConnectionIndicator::off()
{
    m_state = Off;
    setPixmap(KIcon(QString::fromUtf8("network-disconnect")).pixmap(32,32));
}

void ConnectionIndicator::waitingAnimation()
{
    static int index = 0;
    static QList<QPixmap> icons;
    if (icons.isEmpty()) {
        icons.append(KIcon(QString::fromUtf8("network-wireless-disconnected")).pixmap(32,32));
        icons.append(KIcon(QString::fromUtf8("network-wireless-connected-25")).pixmap(32,32));
        icons.append(KIcon(QString::fromUtf8("network-wireless-connected-50")).pixmap(32,32));
        icons.append(KIcon(QString::fromUtf8("network-wireless-connected-75")).pixmap(32,32));
        icons.append(KIcon(QString::fromUtf8("network-wireless")).pixmap(32,32));
    }
    index = (index+1) % icons.count();
    setPixmap(icons[index]);
}

} // namespace RemoteControl
