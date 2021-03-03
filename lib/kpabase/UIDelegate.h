// SPDX-FileCopyrightText: 2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
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
     * @brief Displays a message to the user indicating something went wrong.
     *
     * Additionally, a non-localized log message is logged within the given log category.
     * @param logMsg a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     */
    void sorry(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId = QString());
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
    virtual void showSorry(const QString &msg, const QString &title, const QString &dialogId) = 0;
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
    void showSorry(const QString &, const QString &, const QString &) override { }
    void showError(const QString &, const QString &, const QString &) override { }

public:
    bool isDialogDisabled(const QString &) override { return false; }
};

} // namespace DB

#endif // KPABASE_UIDELEGATE_H
