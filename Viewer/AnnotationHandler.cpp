// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AnnotationHandler.h"
#include "Logging.h"
#include "SelectCategoryAndValue.h"

#include <kpabase/SettingsData.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDebug>
#include <QKeyEvent>
#include <QLocale>
#include <qnamespace.h>

namespace Viewer
{

AnnotationHandler::AnnotationHandler(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

namespace
{
    bool isKeyboardLetter(const QString &txt)
    {
        if (txt.isEmpty())
            return false;

#if 0
        // See https://www.kdab.com/a-little-hidden-gem-qstringiterator/ for details
        // Unfortunately this requires a private header, which our CI doesn't like
        QStringIterator i(txt);
        char32_t ch = i.next();
        return QChar::isLetter(ch);
#else
        return QLocale().toUpper(txt) != txt;
#endif
    }
};

bool AnnotationHandler::handle(QKeyEvent *event)
{
    const auto key = event->text().toLower();

    if (key.isEmpty() || !isKeyboardLetter(key) || (event->modifiers() != Qt::KeyboardModifiers() && event->modifiers() != Qt::KeyboardModifiers(Qt::ShiftModifier)))
        return false;

    if (!m_assignments.contains(key) || event->modifiers().testFlag(Qt::ShiftModifier)) {
        const bool assigned = assignKey(key);
        if (!assigned)
            return false;
    }
    const auto match = m_assignments.value(key);

    Q_EMIT requestToggleCategory(match.category, match.value);

    return true;
}

bool AnnotationHandler::assignKey(const QString &key)
{
    SelectCategoryAndValue dialog(i18nc("@title", "Assign Macro"), i18n("Select item for macro key <b>%1</b>", key), m_assignments);
    connect(&dialog, &SelectCategoryAndValue::helpRequest, this, &AnnotationHandler::requestHelp);
    connect(&dialog, &SelectCategoryAndValue::keyRemovalRequested, this, &AnnotationHandler::clearKey);

    auto result = dialog.exec();
    if (result == QDialog::Rejected)
        return false;

    m_assignments[key] = { dialog.category(), dialog.value() };
    qCDebug(ViewerLog) << "Added macro assignment of key" << key << "to:" << m_assignments.value(key);
    saveSettings();
    return true;
}

void AnnotationHandler::clearKey(const QString &key)
{
    qCDebug(ViewerLog) << "Removed macro assignment of key" << key << "to:" << m_assignments.value(key);
    m_assignments.remove(key);
    saveSettings();
}

namespace
{
    KConfigGroup configGroup()
    {
        const auto section = Settings::SettingsData::instance()->groupForDatabase("viewer keybindings");
        return KSharedConfig::openConfig(QString::fromLatin1("kphotoalbumrc"))->group(section);
    }
}

void AnnotationHandler::saveSettings()
{

    KConfigGroup group = configGroup();
    // delete group so that removed keys are removed:
    group.deleteGroup();
    for (auto it = m_assignments.cbegin(); it != m_assignments.cend(); ++it) {
        auto subgroup = group.group(it.key());
        const auto item = it.value();
        subgroup.writeEntry("category", item.category);
        subgroup.writeEntry("value", item.value);
    }
    group.sync();
}

void AnnotationHandler::loadSettings()
{
    KConfigGroup group = configGroup();
    const QStringList keys = group.groupList();
    for (const QString &key : keys) {
        auto subgroup = group.group(key);
        m_assignments[key] = { subgroup.readEntry("category"), subgroup.readEntry("value") };
    }
}

bool AnnotationHandler::askForTagAndInsert()
{
    SelectCategoryAndValue dialog(i18n("Tag Item"), i18n("Select tag for image"), m_assignments);
    connect(&dialog, &SelectCategoryAndValue::helpRequest, this, &AnnotationHandler::requestHelp);

    auto result = dialog.exec();
    if (result == QDialog::Rejected)
        return false;
    Q_EMIT requestToggleCategory(dialog.category(), dialog.value());
    return true;
}

AnnotationHandler::Assignments AnnotationHandler::assignments() const
{
    return m_assignments;
}

QDebug operator<<(QDebug debug, const AnnotationHandler::Assignment &a)
{
    QDebugStateSaver saveState(debug);
    debug.nospace().noquote() << "\"" << a.category << "/" << a.value << "\"";
    return debug;
}

} // namespace Viewer
