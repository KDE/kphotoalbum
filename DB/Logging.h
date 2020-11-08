/* SPDX-FileCopyrightText: 2017-2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef DB_LOGGING_H
#define DB_LOGGING_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(DBLog)
Q_DECLARE_LOGGING_CATEGORY(DBCategoryMatcherLog)
// log for file loading operations (i.e. by nature very noisy):
Q_DECLARE_LOGGING_CATEGORY(DBFileOpsLog)
Q_DECLARE_LOGGING_CATEGORY(DBImageScoutLog)
Q_DECLARE_LOGGING_CATEGORY(FastDirLog)

#endif /* DB_LOGGING_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
