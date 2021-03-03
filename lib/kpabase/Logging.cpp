/* SPDX-FileCopyrightText: 2017-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "Logging.h"

// only enable warning messages by default
Q_LOGGING_CATEGORY(BaseLog, "kphotoalbum", QtWarningMsg)
Q_LOGGING_CATEGORY(DBLog, "kphotoalbum.DB", QtWarningMsg)
Q_LOGGING_CATEGORY(DBCategoryMatcherLog, "kphotoalbum.DB.CategoryMatcher", QtWarningMsg)
Q_LOGGING_CATEGORY(DBFileOpsLog, "kphotoalbum.DB.FileOperations", QtWarningMsg)
Q_LOGGING_CATEGORY(DBImageScoutLog, "kphotoalbum.DB.ImageScout", QtWarningMsg)
// I'm calling this FastDirLog (and not DBFastDirLog) because FastDir is independent of the rest of DB
Q_LOGGING_CATEGORY(FastDirLog, "kphotoalbum.FastDir", QtWarningMsg)

Q_LOGGING_CATEGORY(ImageManagerLog, "kphotoalbum.ImageManager", QtWarningMsg)

Q_LOGGING_CATEGORY(TimingLog, "kphotoalbum.timingInformation", QtWarningMsg)
Q_LOGGING_CATEGORY(UtilitiesLog, "kphotoalbum.Utilities", QtWarningMsg)
