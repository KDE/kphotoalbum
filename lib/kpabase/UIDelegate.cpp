// SPDX-FileCopyrightText: 2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "UIDelegate.h"

#include <QLoggingCategory>

DB::UserFeedback DB::UIDelegate::warningContinueCancel(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId)
{
    qCWarning(logMsg.category) << logMsg.message;
    return askWarningContinueCancel(msg, title, dialogId);
}

DB::UserFeedback DB::UIDelegate::questionYesNo(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId)
{
    qCInfo(logMsg.category) << logMsg.message;
    return askQuestionYesNo(msg, title, dialogId);
}

void DB::UIDelegate::information(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId)
{
    qCInfo(logMsg.category) << logMsg.message;
    showInformation(msg, title, dialogId);
}

void DB::UIDelegate::error(const LogMessage logMsg, const QString &msg, const QString &title, const QString &dialogId)
{
    qCCritical(logMsg.category) << logMsg.message;
    showError(msg, title, dialogId);
}

int DB::DummyProgressIndicator::minimum() const
{
    return m_min;
}

void DB::DummyProgressIndicator::setMinimum(int min)
{
    m_min = min;
}

int DB::DummyProgressIndicator::maximum() const
{
    return m_max;
}

void DB::DummyProgressIndicator::setMaximum(int max)
{
    m_max = max;
}

int DB::DummyProgressIndicator::value()
{
    return m_value;
}

void DB::DummyProgressIndicator::setValue(int value)
{
    m_value = value;
}

bool DB::DummyProgressIndicator::wasCanceled() const
{
    return m_cancelRequested;
}

void DB::DummyProgressIndicator::setCancelRequested(bool cancel)
{
    m_cancelRequested = cancel;
}
