#ifndef SETTINGS_CALENDARPOPUP_H
#define SETTINGS_CALENDARPOPUP_H

#include <QWidget>
#include <QDate>

class QCalendarWidget;

namespace MainWindow {

class CalendarPopup : public QWidget
{
    Q_OBJECT
public:
    explicit CalendarPopup(QWidget *parent = 0);
    void setSelectedDate(const QDate&);

signals:
    void dateSelected(const QDate& date = QDate());

private:
    void resetDate();

    QCalendarWidget* m_calendar;
};

} // namespace Settings

#endif // SETTINGS_CALENDARPOPUP_H
