/**
 * A date editing widget that consists of an editable combo box.
 * The combo box contains the date in text form, and clicking the combo
 * box arrow will display a 'popup' style date picker.
 *
 * This widget also supports advanced features like allowing the user
 * to type in the day name to get the date. The following keywords
 * are supported (in the native language): tomorrow, yesturday, today,
 * monday, tuesday, wednesday, thursday, friday, saturday, sunday.
 *
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @author Mike Pilone <mpilone@slac.com>
 * @author David Jarvie <software@astrojar.org.uk>
 * @author Jesper Pedersen <blackie@kde.org>
 */
#ifndef ANNOTATIONDIALOG_KDATEEDIT_H
#define ANNOTATIONDIALOG_KDATEEDIT_H

#include <qvbox.h>
#include <qdatetime.h>
#include <qcombobox.h>
#include <qmap.h>
#include "DB/ImageDate.h"

class QEvent;
class KDatePicker;

namespace AnnotationDialog
{

class KDateEdit : public QComboBox
{
    Q_OBJECT
public:
    KDateEdit( bool isStartEdit, QWidget *parent=0, const char *name=0);
    virtual ~KDateEdit();

    /** @return True if the date in the text edit is valid,
     * false otherwise. This will not modify the display of the date,
     * but only check for validity.
     */
    bool inputIsValid() const;

    /** @return The date entered. This will not
     * modify the display of the date, but only return it.
     */
    QDate date() const;

    /** Sets the date.
     *
     * @param date The new date to display. This date must be valid or
     * it will not be displayed.
     */
    void setDate(const QDate& date);

    /** @return The default date used if no valid date has been set or entered.
     */
    QDate defaultDate() const;

    /** Sets the default date to use if no valid date has been set or entered.
     * If no default date has been set, the current date is used as the default.
     * @param date The default date.
     */
    void setDefaultDate(const QDate& date);

    /** @param handleInvalid If true the date edit accepts invalid dates
     * and displays them as the empty ("") string. It also returns an invalid date.
     * If false (default) invalid dates are not accepted and instead the date
     * of today will be returned.
     */
    void setHandleInvalid(bool handleInvalid);

    /** @return True if the widget is accepts invalid dates, false otherwise. */
    bool handlesInvalid() const;

    /** Sets whether the widget is read-only for the user. If read-only, the date
     * picker pop-up is inactive, and the displayed date cannot be edited.
     * @param readOnly True to set the widget read-only, false to set it read-write.
     */
    void setReadOnly(bool readOnly);

    /** @return True if the widget is read-only, false if read-write. */
    bool isReadOnly() const;

    /** Called when a new date has been entered, to validate its value.
     * @param newDate The new date which has been entered.
     * @return True to accept the new date, false to reject the new date.
     * If false is returned, the value reverts to what it was before the
     * new date was entered.
     */
    virtual bool validate(const QDate &newDate);

    virtual void popup();

signals:
    /** This signal is emitted whenever the user modifies the date. This
     * may not get emitted until the user presses enter in the line edit or
     * focus leaves the widget (i.e. the user confirms their selection).
     */
    void dateChanged(QDate);
    void dateChanged( const DB::ImageDate& );

    /** This signal is emitted whenever the user enters an invalid date.
     */
    void invalidDateEntered();

protected slots:
    void dateSelected(QDate);
    void dateEntered(QDate);
    void lineEnterPressed();
    void slotTextChanged(const QString &);
    virtual void mousePressEvent(QMouseEvent *);

private:
    virtual bool eventFilter(QObject *o, QEvent *e);
    bool readDate(QDate& result, QDate* end) const;

    /** Maps the text that the user can enter to the offset in days from
     * today. For example, the text 'tomorrow' is mapped to +1.
     */
    QMap<QString, int> mKeywordMap;
    bool mTextChanged;
    bool mHandleInvalid;

    KDatePicker *mDatePicker;
    QVBox *mDateFrame;
    QDate  defaultValue;
    QDate  value;
    bool   mReadOnly;
    bool   mDiscardNextMousePress;
    bool   mIsStartEdit;
};

}

#endif
