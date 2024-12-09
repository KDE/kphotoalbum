// SPDX-FileCopyrightText: 2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef KPABASE_UIDELEGATE_H
#define KPABASE_UIDELEGATE_H

#include <QString>

namespace DB
{

/**
 * @brief The UserFeedback enum enumerates all possible results of a user interaction.
 */
enum class UserFeedback {
    Confirm ///< The user accepts the dialog, wanting to continue with the action or to say "yes".
    ,
    Deny ///< The user cancels the dialog, not wanting to continue with the action or to say "no".
    ,
    SafeDefaultAction ///< The user did not take a choice (maybe there was no user to interact with in the current context, maybe the user closed the dialog). Most often this should mean the same as Demy, not doing any permanent changes.
};

/**
 * @brief The LogMessage struct combines a log message with its category.
 * The main intention is to make the UIDelegate method signatures more readable (by avoiding repeated QString parameters).
 */
struct LogMessage {
    const QLoggingCategory &category; ///< The logging category that shall be used for logging the message.
    const QString &message; ///< The log message. Log messages should usually not be localized.
};

/**
 * @brief The UIDelegate class encapsulates possible user interaction and feedback.
 * This class is required because in the DB core there should not be any dependency on UI classes.
 * Therefore, the usual routine of calling KMessageBox with some parent widget does not work.
 *
 * The user of the DB core is expected to subclass the pure virtual functions.
 * @see FileReader::setUIDelegate()
 * @see FileWriter::setUIDelegate()
 */
class UIDelegate
{
public:
    /**
     * @brief Similar to KMessageBox::warningContinueCancel, this method displays a message and prompts the user to continue or cancel.
     *
     * Additionally, a non-localized log message is logged within the given log category.
     * @param logMsg a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     * @return the user choice in form of a UserFeedback
     */
    UserFeedback warningContinueCancel(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId = QString());

    /**
     * @brief Similar to KMessageBox::questionYesNo, this method displays a message and prompts the user for a yes/no answer.
     * @param logMsg a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     * @return the user choice in form of a UserFeedback
     */
    UserFeedback questionYesNo(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId = QString());

    /**
     * @brief Displays an informational message to the user.
     *
     * Additionally, a non-localized log message is logged within the given log category.
     * @param logMsg a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     */
    void information(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId = QString());
    /**
     * @brief Displays an error message to the user.
     *
     * Additionally, a non-localized log message is logged within the given log category.
     * @param logMsg a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     */
    void error(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId = QString());

    /**
     * @brief isDialogDisabled checks whether the user disabled display of a dialog.
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     * @return \c true, if the dialog was disabled by the user, \c false otherwise.
     */
    virtual bool isDialogDisabled(const QString &dialogId) = 0;

protected:
    virtual ~UIDelegate() = default;

    virtual UserFeedback askWarningContinueCancel(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual UserFeedback askQuestionYesNo(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual void showInformation(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual void showError(const QString &msg, const QString &title, const QString &dialogId) = 0;
};

/**
 * @brief The DummyUIDelegate class does nothing except returning UserFeedback::DefaultAction on questions.
 */
class DummyUIDelegate : public UIDelegate
{
protected:
    UserFeedback askWarningContinueCancel(const QString &, const QString &, const QString &) override { return UserFeedback::SafeDefaultAction; }
    UserFeedback askQuestionYesNo(const QString &, const QString &, const QString &) override { return UserFeedback::SafeDefaultAction; }
    void showInformation(const QString &, const QString &, const QString &) override { }
    void showError(const QString &, const QString &, const QString &) override { }

public:
    bool isDialogDisabled(const QString &) override { return false; }
};

/**
 * @brief The AbstractProgressIndicator class provides a generic progress indicator interface.
 * It is mostly designed as a UI-agnostic interface that allows a drop-in replacement for QProgressDialog.
 */
class AbstractProgressIndicator
{
public:
    virtual int minimum() const = 0;
    virtual void setMinimum(int min) = 0;
    virtual int maximum() const = 0;
    virtual void setMaximum(int max) = 0;
    virtual int value() = 0;
    virtual void setValue(int value) = 0;

    /**
     * @brief wasCanceled signals whether a cancellation of the action was requested.
     * @return \c true, if the action shall be aborted, or \c false if it shall continue.
     */
    virtual bool wasCanceled() const = 0;

protected:
    virtual ~AbstractProgressIndicator() { };
};

template <class Super>
class ProgressDialog : public DB::AbstractProgressIndicator, public Super
{
public:
    int minimum() const override
    {
        return Super::minimum();
    }
    void setMinimum(int min) override
    {
        Super::setMinimum(min);
    }
    int maximum() const override
    {
        return Super::maximum();
    }
    void setMaximum(int max) override
    {
        Super::setMaximum(max);
    }
    int value() override
    {
        return Super::value();
    }
    void setValue(int value) override
    {
        Super::setValue(value);
    }
    bool wasCanceled() const override
    {
        return Super::wasCanceled();
    }
};

class DummyProgressIndicator : public DB::AbstractProgressIndicator
{
public:
    int minimum() const override;
    void setMinimum(int min) override;
    int maximum() const override;
    void setMaximum(int max) override;
    int value() override;
    void setValue(int value) override;
    bool wasCanceled() const override;

    void setCancelRequested(bool cancel);

private:
    int m_min = 0;
    int m_max = 100;
    int m_value = 0;
    bool m_cancelRequested = false;
};
} // namespace DB

#endif // KPABASE_UIDELEGATE_H
