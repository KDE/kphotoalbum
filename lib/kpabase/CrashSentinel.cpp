// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
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
}

KPABase::CrashSentinel::CrashSentinel(const QString &component, const QString &crashInfo)
    : m_component(component)
    , m_crashInfo(crashInfo)
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    m_lastCrashInfo = cfgGroup.readEntry(m_component, QString());
    if (!m_lastCrashInfo.isEmpty()) {
        const auto historyEntry = m_component + QString::fromUtf8(CFG_HISTORY);
        auto history = cfgGroup.readEntry(historyEntry, QStringList());
        history.append(m_lastCrashInfo);
        cfgGroup.writeEntry(historyEntry, history);
    }
    qCDebug(BaseLog).nospace() << "Created CrashSentinel for component " << m_component << ". Previous crash information: "
                               << lastCrashInfo() << (hasCrashInfo() ? "" : "n/a");
}

KPABase::CrashSentinel::~CrashSentinel()
{
    suspend();
}

bool KPABase::CrashSentinel::hasCrashInfo() const
{
    return !m_lastCrashInfo.isEmpty();
}

QString KPABase::CrashSentinel::lastCrashInfo() const
{
    return m_lastCrashInfo;
}

QStringList KPABase::CrashSentinel::crashHistory() const
{
    const auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    return cfgGroup.readEntry(m_component + QString::fromUtf8(CFG_HISTORY), QStringList());
}

void KPABase::CrashSentinel::clearCrashHistory()
{
    auto cfgGroup = KSharedConfig::openConfig()->group(CFG_GROUP);
    cfgGroup.deleteEntry(m_component + QString::fromUtf8(CFG_HISTORY));
}

void KPABase::CrashSentinel::setCrashInfo(const QString &crashInfo)
{
    suspend();
    m_crashInfo = crashInfo;
    activate();
}

QString KPABase::CrashSentinel::component() const
{
    return m_component;
}

QString KPABase::CrashSentinel::crashInfo() const
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

// vi:expandtab:tabstop=4 shiftwidth=4:
