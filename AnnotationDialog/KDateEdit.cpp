/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include "KDateEdit.h"

#include <qevent.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <QMouseEvent>
#include <QKeyEvent>

#include <kdatepicker.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <kdebug.h>

#include "KDateEdit.moc"
#include <QVBoxLayout>

AnnotationDialog::KDateEdit::KDateEdit( bool isStartEdit, QWidget *parent )
    : KComboBox( parent ),
      defaultValue( QDate::currentDate() ),
      mReadOnly(false),
      mDiscardNextMousePress(false),
      mIsStartEdit( isStartEdit )
{
    setEditable(true);
    setMaxCount(1);       // need at least one entry for popup to work
    value = defaultValue;
    QString today = QDate::currentDate().toString( QString::fromLatin1("dd. MMM yyyy") );
    addItem(QString::fromLatin1( "" ) );
    setCurrentIndex(0);
    setItemText( 0, QString::fromLatin1( "" ));
    setMinimumSize(sizeHint());

    mDateFrame = new QFrame;
    mDateFrame->setWindowFlags(Qt::Popup);
    QVBoxLayout* layout = new QVBoxLayout(mDateFrame);
    mDateFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    mDateFrame->setLineWidth(3);
    mDateFrame->hide();
    mDateFrame->installEventFilter(this);

    mDatePicker = new KDatePicker(value, mDateFrame);
    layout->addWidget(mDatePicker);

    connect(lineEdit(),SIGNAL(editingFinished()),SLOT(lineEnterPressed()));
    connect(this,SIGNAL(textChanged(QString)),
            SLOT(slotTextChanged(QString)));

    connect(mDatePicker,SIGNAL(dateEntered(QDate)),SLOT(dateEntered(QDate)));
    connect(mDatePicker,SIGNAL(dateSelected(QDate)),SLOT(dateSelected(QDate)));

    // Create the keyword list. This will be used to match against when the user
    // enters information.
    mKeywordMap[i18n("tomorrow")] = 1;
    mKeywordMap[i18n("today")] = 0;
    mKeywordMap[i18n("yesterday")] = -1;

    QString dayName;
    for (int i = 1; i <= 7; ++i)
    {
        dayName = KGlobal::locale()->calendar()->weekDayName(i).toLower();
        mKeywordMap[dayName] = i + 100;
    }
    lineEdit()->installEventFilter(this);   // handle keyword entry

    mTextChanged = false;
    mHandleInvalid = false;
}

AnnotationDialog::KDateEdit::~KDateEdit()
{
}

void AnnotationDialog::KDateEdit::setDate(const QDate& newDate)
{
    QString dateString = QString::fromLatin1("");
    if(newDate.isValid())
        dateString = DB::ImageDate( newDate ).toString( false );

    mTextChanged = false;

    // We do not want to generate a signal here, since we explicitly setting
    // the date
    bool b = signalsBlocked();
    blockSignals(true);
    setItemText(0, dateString);
    blockSignals(b);

    value = newDate;
}

void AnnotationDialog::KDateEdit::setHandleInvalid(bool handleInvalid)
{
    mHandleInvalid = handleInvalid;
}

bool AnnotationDialog::KDateEdit::handlesInvalid() const
{
    return mHandleInvalid;
}

void AnnotationDialog::KDateEdit::setReadOnly(bool readOnly)
{
    mReadOnly = readOnly;
    lineEdit()->setReadOnly(readOnly);
}

bool AnnotationDialog::KDateEdit::isReadOnly() const
{
    return mReadOnly;
}

bool AnnotationDialog::KDateEdit::validate( const QDate & )
{
    return true;
}

QDate AnnotationDialog::KDateEdit::date() const
{
    QDate dt;
    readDate(dt, 0);
    return dt;
}

QDate AnnotationDialog::KDateEdit::defaultDate() const
{
    return defaultValue;
}

void AnnotationDialog::KDateEdit::setDefaultDate(const QDate& date)
{
    defaultValue = date;
}

void AnnotationDialog::KDateEdit::showPopup()
{
    if (mReadOnly)
        return;

    QRect desk = KGlobalSettings::desktopGeometry(this);

    QPoint popupPoint = mapToGlobal( QPoint( 0,0 ) );
    if ( popupPoint.x() < desk.left() ) popupPoint.setX( desk.x() );

    int dateFrameHeight = mDateFrame->sizeHint().height();

    if ( popupPoint.y() + height() + dateFrameHeight > desk.bottom() ) {
        popupPoint.setY( popupPoint.y() - dateFrameHeight );
    } else {
        popupPoint.setY( popupPoint.y() + height() );
    }

    mDateFrame->move( popupPoint );

    QDate date;
    readDate(date, 0);
    if (date.isValid()) {
        mDatePicker->setDate( date );
    } else {
        mDatePicker->setDate( defaultValue );
    }

    mDateFrame->show();
}

void AnnotationDialog::KDateEdit::dateSelected(QDate newDate)
{
    if ((mHandleInvalid || newDate.isValid()) && validate(newDate)) {
        setDate(newDate);
        emit dateChanged(newDate);
        emit dateChanged( DB::ImageDate( QDateTime(newDate), QDateTime(newDate) ) );
        mDateFrame->hide();
    }
}

void AnnotationDialog::KDateEdit::dateEntered(QDate newDate)
{
    if ((mHandleInvalid || newDate.isValid()) && validate(newDate)) {
        setDate(newDate);
        emit dateChanged(newDate);
        emit dateChanged( DB::ImageDate( QDateTime(newDate), QDateTime(newDate) ) );
    }
}

void AnnotationDialog::KDateEdit::lineEnterPressed()
{
    if ( !mTextChanged )
        return;

    QDate date;
    QDate end;
    if (readDate(date, &end) && (mHandleInvalid || date.isValid()) && validate(date))
    {
        // Update the edit. This is needed if the user has entered a
        // word rather than the actual date.
        setDate(date);
        emit(dateChanged(date));
        emit dateChanged( DB::ImageDate( QDateTime(date), QDateTime(end) ) );
    }
    else {
        // Invalid or unacceptable date - revert to previous value
        setDate(value);
        emit invalidDateEntered();
    }
}

bool AnnotationDialog::KDateEdit::inputIsValid() const
{
    QDate date;
    return readDate(date, 0) && date.isValid();
}

/* Reads the text from the line edit. If the text is a keyword, the
 * word will be translated to a date. If the text is not a keyword, the
 * text will be interpreted as a date.
 * Returns true if the date text is blank or valid, false otherwise.
 */
bool AnnotationDialog::KDateEdit::readDate(QDate& result, QDate* end) const
{
    QString text = currentText();

    if (text.isEmpty()) {
        result = QDate();
    }
    else if (mKeywordMap.contains(text.toLower()))
    {
        QDate today = QDate::currentDate();
        int i = mKeywordMap[text.toLower()];
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
        result = DB::ImageDate::parseDate( text, mIsStartEdit );
        if ( end )
            *end = DB::ImageDate::parseDate( text, false );
        return result.isValid();
    }

    return true;
}

void AnnotationDialog::KDateEdit::keyPressEvent( QKeyEvent *event )
{
    int step = 0;
 
    if ( event->key() == Qt::Key_Up )
        step = 1;
    else if ( event->key() == Qt::Key_Down )
        step = -1;
 
    setDate( value.addDays(step) );
    KComboBox::keyPressEvent( event );
}

/* Checks for a focus out event. The display of the date is updated
 * to display the proper date when the focus leaves.
 */
bool AnnotationDialog::KDateEdit::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == lineEdit()) {
        if (e->type() == QEvent::Wheel) {
            // Up and down arrow keys step the date
            QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
            Q_ASSERT( we != nullptr );

            int step = 0;
            step = we->delta() > 0 ? 1 : -1;
            if (we->orientation() == Qt::Vertical) {
                setDate( value.addDays(step) );
            }
        }
    }
    else {
        // It's a date picker event
        switch (e->type()) {
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress: {
            QMouseEvent *me = (QMouseEvent*)e;
            if (!mDateFrame->rect().contains(me->pos())) {
                QPoint globalPos = mDateFrame->mapToGlobal(me->pos());
                if (QApplication::widgetAt(globalPos) == this) {
                    // The date picker is being closed by a click on the
                    // KDateEdit widget. Avoid popping it up again immediately.
                    mDiscardNextMousePress = true;
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

void AnnotationDialog::KDateEdit::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton  &&  mDiscardNextMousePress) {
        mDiscardNextMousePress = false;
        return;
    }
    KComboBox::mousePressEvent(e);
}

void AnnotationDialog::KDateEdit::slotTextChanged(const QString &)
{
    mTextChanged = true;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
