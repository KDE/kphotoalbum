/* SPDX-FileCopyrightText: 2017-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KPABASE_LOGGING_H
#define KPABASE_LOGGING_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BaseLog)
Q_DECLARE_LOGGING_CATEGORY(DBLog)
Q_DECLARE_LOGGING_CATEGORY(DBCategoryMatcherLog)
// log for file loading operations (i.e. by nature very noisy):
Q_DECLARE_LOGGING_CATEGORY(DBFileOpsLog)
Q_DECLARE_LOGGING_CATEGORY(DBImageScoutLog)
Q_DECLARE_LOGGING_CATEGORY(ExifLog)
Q_DECLARE_LOGGING_CATEGORY(FastDirLog)
Q_DECLARE_LOGGING_CATEGORY(ImageManagerLog)
Q_DECLARE_LOGGING_CATEGORY(TimingLog)
Q_DECLARE_LOGGING_CATEGORY(UtilitiesLog)

#endif /* KPABASE_LOGGING_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
