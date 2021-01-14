/* SPDX-FileCopyrightText: 2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "UIDelegate.h"

#include "Logging.h"

#include <QLoggingCategory>

DB::UserFeedback DB::UIDelegate::warningContinueCancel(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId)
{
    qCWarning(DBLog) << logMessage;
    return askWarningContinueCancel(msg, title, dialogId);
}

DB::UserFeedback DB::UIDelegate::questionYesNo(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId)
{
    qCInfo(DBLog) << logMessage;
    return askQuestionYesNo(msg, title, dialogId);
}

void DB::UIDelegate::information(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId)
{
    qCInfo(DBLog) << logMessage;
    showInformation(msg, title, dialogId);
}

void DB::UIDelegate::sorry(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId)
{
    qCWarning(DBLog) << logMessage;
    showSorry(msg, title, dialogId);
}

void DB::UIDelegate::error(const QString &logMessage, const QString &msg, const QString &title, const QString &dialogId)
{
    qCCritical(DBLog) << logMessage;
    showError(msg, title, dialogId);
}
