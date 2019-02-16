/* Copyright (C) 2019 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UIDELEGATE_H
#define UIDELEGATE_H

#include <QString>


namespace DB {

/**
 * @brief The UIFeedback enum enumerates all possible results of a user interaction.
 */
enum class UIFeedback {
    Continue ///< The user accepts the dialog, wanting to continue with the action.
    , Cancel ///< The user cancels the dialog, not wanting to continue with the action.
    , DefaultAction ///< The user did not take a choice (maybe there was no user to interact with in the current context). Usually this should mean the same as Cancel, not doing any permanent changes.
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
     * Additionally, a non-localized logMessage is logged within the DB log category.
     * @param logMessage a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     * @return the user choice in form of a UIFeedback
     */
    UIFeedback warningContinueCancel(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId = QString());

    /**
     * @brief Similar to KMessageBox::questionYesNo, this method displays a message and prompts the user for a yes/no answer.
     * @param logMessage a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     * @return the user choice in form of a UIFeedback
     */
    UIFeedback questionYesNo(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId = QString());

    /**
     * @brief Displays an informational message to the user.
     *
     * Additionally, a non-localized logMessage is logged within the DB log category.
     * @param logMessage a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     */
    void information(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId = QString());
    /**
     * @brief Displays a message to the user indicating something went wrong.
     *
     * Additionally, a non-localized logMessage is logged within the DB log category.
     * @param logMessage a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     */
    void sorry(const QString &msg, const QString &logMessage, const QString &title, const QString &dialogId = QString());
    /**
     * @brief Displays an error message to the user.
     *
     * Additionally, a non-localized logMessage is logged within the DB log category.
     * @param logMessage a non-localized log message
     * @param msg a localized message
     * @param title a localized title for a possible message window
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     */
    void error(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId = QString());

    /**
     * @brief isDialogDisabled checks whether the user disabled display of a dialog.
     * @param dialogId an ID to identify the dialog (can be used to give the user a "don't ask again" checkbox)
     * @return \c true, if the dialog was disabled by the user, \c false otherwise.
     */
    virtual bool isDialogDisabled(const QString &dialogId) = 0;
protected:
    virtual ~UIDelegate() = default;

    virtual UIFeedback askWarningContinueCancel(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual UIFeedback askQuestionYesNo(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual void showInformation(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual void showSorry(const QString &msg, const QString &title, const QString &dialogId) = 0;
    virtual void showError(const QString &msg, const QString &title, const QString &dialogId) = 0;
};

/**
 * @brief The DummyUIDelegate class does nothing except returning UIFeedback::DefaultAction on questions.
 */
class DummyUIDelegate : public UIDelegate
{
protected:
    UIFeedback askWarningContinueCancel(const QString &, const QString &, const QString &) override {return UIFeedback::DefaultAction; }
    UIFeedback askQuestionYesNo(const QString &, const QString &, const QString &) override {return UIFeedback::DefaultAction; }
    void showInformation(const QString &, const QString &, const QString &) override {}
    void showSorry(const QString &, const QString &, const QString &) override {}
    void showError(const QString &, const QString &, const QString &) override {}

public:
    bool isDialogDisabled(const QString &) override { return false; }
};

} // namespace DB

#endif // UIDELEGATE_H
