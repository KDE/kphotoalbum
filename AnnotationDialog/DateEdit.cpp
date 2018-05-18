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

/**
 * A date editing widget that consists of an editable combo box.
 * The combo box contains the date in text form, and clicking the combo
 * box arrow will display a 'popup' style date picker.
 *
 * This widget also supports advanced features like allowing the user
 * to type in the day name to get the date. The following keywords
 * are supported (in the native language): tomorrow, yesterday, today,
 * monday, tuesday, wednesday, thursday, friday, saturday, sunday.
 *
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @author Mike Pilone <mpilone@slac.com>
 * @author David Jarvie <software@astrojar.org.uk>
 * @author Jesper Pedersen <blackie@kde.org>
 */

#include "DateEdit.h"

#include <KDatePicker>
#include <KLocalizedString>

#include <QApplication>
#include <QDate>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QVBoxLayout>

AnnotationDialog::DateEdit::DateEdit( bool isStartEdit, QWidget *parent )
    : QComboBox( parent ),
      m_defaultValue( QDate::currentDate() ),
      m_ReadOnly(false),
      m_DiscardNextMousePress(false),
      m_IsStartEdit( isStartEdit )
{
    setEditable(true);
    setMaxCount(1);       // need at least one entry for popup to work
    m_value = m_defaultValue;
    QString today = QDate::currentDate().toString( QString::fromLatin1("dd. MMM yyyy") );
    addItem(QString::fromLatin1( "" ) );
    setCurrentIndex(0);
    setItemText( 0, QString::fromLatin1( "" ));
    setMinimumSize(sizeHint());

    m_DateFrame = new QFrame;
    m_DateFrame->setWindowFlags(Qt::Popup);
    QVBoxLayout* layout = new QVBoxLayout(m_DateFrame);
    m_DateFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    m_DateFrame->setLineWidth(3);
    m_DateFrame->hide();
    m_DateFrame->installEventFilter(this);

    m_DatePicker = new KDatePicker(m_value, m_DateFrame);
    layout->addWidget(m_DatePicker);

    connect(lineEdit(),&QLineEdit::editingFinished,this,&DateEdit::lineEnterPressed);
    connect(this,&QComboBox::currentTextChanged,this,&DateEdit::slotTextChanged);

    connect(m_DatePicker, &KDatePicker::dateEntered, this, &DateEdit::dateEntered);
    connect(m_DatePicker, &KDatePicker::dateSelected, this, &DateEdit::dateSelected);

    // Create the keyword list. This will be used to match against when the user
    // enters information.
    m_KeywordMap[i18n("tomorrow")] = 1;
    m_KeywordMap[i18n("today")] = 0;
    m_KeywordMap[i18n("yesterday")] = -1;

    QString dayName;
    for (int i = 1; i <= 7; ++i)
    {
        dayName = QLocale().dayName(i,QLocale::LongFormat).toLower();
        m_KeywordMap[dayName] = i + 100;
    }
    lineEdit()->installEventFilter(this);   // handle keyword entry

    m_TextChanged = false;
    m_HandleInvalid = false;
}

AnnotationDialog::DateEdit::~DateEdit()
{
}

void AnnotationDialog::DateEdit::setDate(const QDate& newDate)
{
    QString dateString = QString::fromLatin1("");
    if(newDate.isValid())
        dateString = DB::ImageDate( newDate ).toString( false );

    m_TextChanged = false;

    // We do not want to generate a signal here, since we explicitly setting
    // the date
    bool b = signalsBlocked();
    blockSignals(true);
    setItemText(0, dateString);
    blockSignals(b);

    m_value = newDate;
}

void AnnotationDialog::DateEdit::setHandleInvalid(bool handleInvalid)
{
    m_HandleInvalid = handleInvalid;
}

bool AnnotationDialog::DateEdit::handlesInvalid() const
{
    return m_HandleInvalid;
}

void AnnotationDialog::DateEdit::setReadOnly(bool readOnly)
{
    m_ReadOnly = readOnly;
    lineEdit()->setReadOnly(readOnly);
}

bool AnnotationDialog::DateEdit::isReadOnly() const
{
    return m_ReadOnly;
}

bool AnnotationDialog::DateEdit::validate( const QDate & )
{
    return true;
}

QDate AnnotationDialog::DateEdit::date() const
{
    QDate dt;
    readDate(dt, 0);
    return dt;
}

QDate AnnotationDialog::DateEdit::defaultDate() const
{
    return m_defaultValue;
}

void AnnotationDialog::DateEdit::setDefaultDate(const QDate& date)
{
    m_defaultValue = date;
}

void AnnotationDialog::DateEdit::showPopup()
{
    if (m_ReadOnly)
        return;

    QRect desk = QApplication::desktop()->availableGeometry(this);

    // ensure that the popup is fully visible even when the DateEdit is off-screen
    QPoint popupPoint = mapToGlobal( QPoint( 0,0 ) );
    if ( popupPoint.x() < desk.left() ) {
        popupPoint.setX( desk.x() );
    } else if ( popupPoint.x() + width() > desk.right()) {
        popupPoint.setX( desk.right() - width());
    }
    int dateFrameHeight = m_DateFrame->sizeHint().height();
    if ( popupPoint.y() + height() + dateFrameHeight > desk.bottom() ) {
        popupPoint.setY( popupPoint.y() - dateFrameHeight );
    } else {
        popupPoint.setY( popupPoint.y() + height() );
    }

    m_DateFrame->move( popupPoint );

    QDate date;
    readDate(date, 0);
    if (date.isValid()) {
        m_DatePicker->setDate( date );
    } else {
        m_DatePicker->setDate( m_defaultValue );
    }

    m_DateFrame->show();
}

void AnnotationDialog::DateEdit::dateSelected(QDate newDate)
{
    if ((m_HandleInvalid || newDate.isValid()) && validate(newDate)) {
        setDate(newDate);
        emit dateChanged(newDate);
        emit dateChanged( DB::ImageDate( QDateTime(newDate), QDateTime(newDate) ) );
        m_DateFrame->hide();
    }
}

void AnnotationDialog::DateEdit::dateEntered(QDate newDate)
{
    if ((m_HandleInvalid || newDate.isValid()) && validate(newDate)) {
        setDate(newDate);
        emit dateChanged(newDate);
        emit dateChanged( DB::ImageDate( QDateTime(newDate), QDateTime(newDate) ) );
    }
}

void AnnotationDialog::DateEdit::lineEnterPressed()
{
    if ( !m_TextChanged )
        return;

    QDate date;
    QDate end;
    if (readDate(date, &end) && (m_HandleInvalid || date.isValid()) && validate(date))
    {
        // Update the edit. This is needed if the user has entered a
        // word rather than the actual date.
        setDate(date);
        emit(dateChanged(date));
        emit dateChanged( DB::ImageDate( QDateTime(date), QDateTime(end) ) );
    }
    else {
        // Invalid or unacceptable date - revert to previous value
        setDate(m_value);
        emit invalidDateEntered();
    }
}

bool AnnotationDialog::DateEdit::inputIsValid() const
{
    QDate date;
    return readDate(date, 0) && date.isValid();
}

/* Reads the text from the line edit. If the text is a keyword, the
 * word will be translated to a date. If the text is not a keyword, the
 * text will be interpreted as a date.
 * Returns true if the date text is blank or valid, false otherwise.
 */
bool AnnotationDialog::DateEdit::readDate(QDate& result, QDate* end) const
{
    QString text = currentText();

    if (text.isEmpty()) {
        result = QDate();
    }
    else if (m_KeywordMap.contains(text.toLower()))
    {
        QDate today = QDate::currentDate();
        int i = m_KeywordMap[text.toLower()];
        if (i >= 100)
        {
            /* A day name has been entered. Convert to offset from today.
             * This uses some math tricks to figure out the offset in days
             * to the next date the given day of the week occurs. There
             * are two cases, that the new day is >= the current day, which means
             * the new day has not occurred yet or that the new day < the current day,
             * which means the new day is already passed (so we need to find the
             * day in the next week).
             */
            i -= 100;
            int currentDay = today.dayOfWeek();
            if (i >= currentDay)
                i -= currentDay;
            else
                i += 7 - currentDay;
        }
        result = today.addDays(i);
    }
    else
    {
        result = DB::ImageDate::parseDate( text, m_IsStartEdit );
        if ( end )
            *end = DB::ImageDate::parseDate( text, false );
        return result.isValid();
    }

    return true;
}

void AnnotationDialog::DateEdit::keyPressEvent( QKeyEvent *event )
{
    int step = 0;

    if ( event->key() == Qt::Key_Up )
        step = 1;
    else if ( event->key() == Qt::Key_Down )
        step = -1;

    setDate( m_value.addDays(step) );
    QComboBox::keyPressEvent( event );
}

/* Checks for a focus out event. The display of the date is updated
 * to display the proper date when the focus leaves.
 */
bool AnnotationDialog::DateEdit::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == lineEdit()) {
        if (e->type() == QEvent::Wheel) {
            // Up and down arrow keys step the date
            QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
            Q_ASSERT( we != nullptr );

            int step = 0;
            step = we->delta() > 0 ? 1 : -1;
            if (we->orientation() == Qt::Vertical) {
                setDate( m_value.addDays(step) );
            }
        }
    }
    else {
        // It's a date picker event
        switch (e->type()) {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress: {
            QMouseEvent *me = (QMouseEvent*)e;
            if (!m_DateFrame->rect().contains(me->pos())) {
                QPoint globalPos = m_DateFrame->mapToGlobal(me->pos());
                if (QApplication::widgetAt(globalPos) == this) {
                    // The date picker is being closed by a click on the
                    // DateEdit widget. Avoid popping it up again immediately.
                    m_DiscardNextMousePress = true;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    return false;
}

void AnnotationDialog::DateEdit::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton  &&  m_DiscardNextMousePress) {
        m_DiscardNextMousePress = false;
        return;
    }
    QComboBox::mousePressEvent(e);
}

void AnnotationDialog::DateEdit::slotTextChanged(const QString &)
{
    m_TextChanged = true;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
