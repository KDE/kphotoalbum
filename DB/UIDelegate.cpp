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
