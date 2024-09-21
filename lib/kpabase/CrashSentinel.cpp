// SPDX-FileCopyrightText: 2021 - 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "CrashSentinel.h"
#include "Logging.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <QLoggingCategory>

namespace
{
constexpr auto CFG_GROUP { "CrashInfo" };
constexpr auto CFG_HISTORY { "_crashHistory" };
constexpr auto CFG_DISABLED { "_disabled" };
}

KPABase::CrashSentinel::CrashSentinel(const QString &component, const QByteArray &crashInfo)
    : m_component(component)
    , m_crashInfo(crashInfo)
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    m_lastCrashInfo = cfgGroup.readEntry(m_component, QByteArray());
    if (!m_lastCrashInfo.isEmpty()) {
        const QString historyEntry = m_component + QString::fromUtf8(CFG_HISTORY);
        auto history = cfgGroup.readEntry(historyEntry, QList<QByteArray>());
        history.append(m_lastCrashInfo);
        cfgGroup.writeEntry(historyEntry, history);
    }
    qCDebug(BaseLog).nospace() << "Created CrashSentinel for component " << m_component << ". Previous crash information: "
                               << m_lastCrashInfo << (isDisabled() ? "; crash detection was permanently disabled." : "; crash detection is active.");
}

KPABase::CrashSentinel::~CrashSentinel()
{
    suspend();
}

bool KPABase::CrashSentinel::hasCrashInfo() const
{
    if (isDisabled())
        return false;
    return !m_lastCrashInfo.isEmpty();
}

QByteArray KPABase::CrashSentinel::lastCrashInfo() const
{
    if (isDisabled())
        return {};
    return m_lastCrashInfo;
}

QList<QByteArray> KPABase::CrashSentinel::crashHistory() const
{
    if (isDisabled())
        return {};
    const auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    return cfgGroup.readEntry(m_component + QString::fromUtf8(CFG_HISTORY), QList<QByteArray>());
}

void KPABase::CrashSentinel::clearCrashHistory()
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    cfgGroup.deleteEntry(m_component + QString::fromUtf8(CFG_HISTORY));
    cfgGroup.deleteEntry(m_component + QString::fromUtf8(CFG_DISABLED));
}

void KPABase::CrashSentinel::setCrashInfo(const QByteArray &crashInfo)
{
    const bool wasActive = !isSuspended();
    suspend();
    m_crashInfo = crashInfo;
    if (wasActive) {
        activate();
    }
}

QString KPABase::CrashSentinel::component() const
{
    return m_component;
}

QByteArray KPABase::CrashSentinel::crashInfo() const
{
    return m_crashInfo;
}

bool KPABase::CrashSentinel::isSuspended() const
{
    const auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    return !cfgGroup.hasKey(m_component);
}

void KPABase::CrashSentinel::suspend()
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    cfgGroup.deleteEntry(m_component);
    cfgGroup.sync();
    qCDebug(BaseLog) << "CrashSentinel for component" << m_component << "suspended.";
}

void KPABase::CrashSentinel::activate()
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    cfgGroup.writeEntry(m_component, m_crashInfo);
    cfgGroup.sync();
    qCDebug(BaseLog) << "CrashSentinel for component" << m_component << "activated. Crash info:" << m_crashInfo;
}

void KPABase::CrashSentinel::disablePermanently()
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    cfgGroup.writeEntry(m_component + QString::fromUtf8(CFG_DISABLED), true);
    cfgGroup.sync();
    qCDebug(BaseLog) << "CrashSentinel for component" << m_component << "permanently disabled.";
}

bool KPABase::CrashSentinel::isDisabled() const
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    return cfgGroup.readEntry(m_component + QString::fromUtf8(CFG_DISABLED), false);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
