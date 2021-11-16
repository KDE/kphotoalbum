// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef KPABASE_CRASH_SENTINEL_H
#define KPABASE_CRASH_SENTINEL_H

#include <QString>

namespace KPABase
{

/**
 * @brief The CrashSentinel class acts as a memento as to what KPhotoAlbum was doing when a crash occurs.
 * The intention is to use this when using external libraries that are known to crash sometimes and that are outside of our control.
 *
 * The CrashSentinel was introduced to be able to react to some video backends crashing for some users.
 * If the app knows that a video was playing when while it exited anomally, it can try a different backend next time.
 */
class CrashSentinel
{
public:
    /**
     * @brief Create a CrashSentinel for the given component.
     * The sentinel writes the crashInfo into the `<component>Sentinel` configuration of the `[CrashInfo]` section of the application configuration file.
     * When the CrashSentinel is deleted or if suspend() is called, the crashInfo is removed. Calling resume() reenables the crash detection.
     * @param component the name of the component that is covered by the CrashSentinel
     * @param crashInfo the identifier of the currently active code path
     */
    CrashSentinel(const QString &component, const QString &crashInfo);
    ~CrashSentinel();

    /**
     * @brief hasCrashInfo
     * @return \c true, if there was a crash info upon initialization of the sentinel, \c false if not.
     */
    bool hasCrashInfo() const;
    /**
     * @brief lastCrashInfo
     * @return the crash information from the last crash, or an empty string if hasCrashInfo() is false.
     */
    QString lastCrashInfo() const;

    /**
     * @brief crashHistory
     * @return a list of all previously recorded crash infos (including the lastCrashInfo)
     */
    QStringList crashHistory() const;

    /**
     * @brief clearCrashHistory clears all previously recorded crash infos.
     * The information returned by hasCrashInfo() and lastCrashInfo() is not affected.
     */
    void clearCrashHistory();

    /**
     * @brief setCrashInfo replaces the crashInfo with a new identifier.
     * Setting the crashInfo does not affect whether the sentinel is suspended or not.
     */
    void setCrashInfo(const QString &crashInfo);

    /**
     * @brief crashInfo
     * @return the active crashInfo identifier.
     */
    QString crashInfo() const;

    /**
     * @brief component
     * @return the component name.
     */
    QString component() const;

    /**
     * @brief isSuspended
     * @return \c true, if the sentinel is currently suspended, \c false otherwise
     */
    bool isSuspended() const;

    /**
     * @brief suspend the CrashSentinel.
     * While suspended, the sentinel will not be able to register a crash.
     * Call this, if the "crashy" component is unlikely to actually crash, e.g. when the component is still loaded in memory but not doing anything.
     * This allows you to avoid false positives because of the application crashing in another component.
     */
    void suspend();

    /**
     * @brief resume the CrashSentinel if it is suspended.
     * Calling resume on a non-suspended CrashSentinel will do nothing.
     */
    void resume();

private:
    const QString m_component;
    QString m_crashInfo;
    QString m_lastCrashInfo;
};

}

#endif // KPABASE_CRASH_SENTINEL_H
// vi:expandtab:tabstop=4 shiftwidth=4:
