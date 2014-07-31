#include "CalendarPopup.h"

#include <QCalendarWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <KLocale>

namespace MainWindow {

CalendarPopup::CalendarPopup(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(Qt::Popup);

    QVBoxLayout* vlay = new QVBoxLayout(this);

    m_calendar = new QCalendarWidget;
    vlay->addWidget(m_calendar);

    QPushButton* button = new QPushButton(i18n("unset"));
    vlay->addWidget(button);

    connect(m_calendar, SIGNAL(clicked(QDate)), this, SIGNAL(dateSelected(QDate)));
    connect(button, SIGNAL(clicked()), this, SIGNAL(dateSelected()));
}

void CalendarPopup::setSelectedDate(const QDate& date)
{
    m_calendar->setSelectedDate(date);
}

} // namespace Settings
