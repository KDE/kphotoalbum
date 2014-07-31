#ifndef SETTINGS_BIRTHDAYPAGE_H
#define SETTINGS_BIRTHDAYPAGE_H

#include <QDialog>
class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QScrollArea;
class QDate;
class QLineEdit;

namespace MainWindow {
class CalendarPopup;

class BirthDatesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BirthDatesDialog(QWidget *parent = 0);
    QSize sizeHint() const;

private slots:
    void changeCategory(int index);
    void resetCategory();
    void editDate();
    void setDate(const QDate&);

private:
    QString textForDate(const QDate& date) const;
    QComboBox* m_categoryBox;
    QScrollArea* m_area;
    CalendarPopup* m_datePick = nullptr;
    QPushButton* m_editButton;
    QLineEdit* m_filter;
};

} // namespace Settings

#endif // SETTINGS_BIRTHDAYPAGE_H
