// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AnnotationHandler.h"
#include "SelectCategoryAndValue.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDebug>
#include <QKeyEvent>
#include <QLocale>
#include <private/qstringiterator_p.h>
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
    // See https://www.kdab.com/a-little-hidden-gem-qstringiterator/ for details
    bool isKeyboardLetter(const QString &txt)
    {
        if (txt.isEmpty())
            return false;

        QStringIterator i(txt);
        char32_t ch = i.next();
        return QChar::isLetter(ch);
    }
};

bool AnnotationHandler::handle(QKeyEvent *event)
{
    const auto key = event->text().toLower();

    if (key.isEmpty() || !isKeyboardLetter(key) || (event->modifiers() != Qt::KeyboardModifiers() && event->modifiers() != Qt::KeyboardModifiers(Qt::ShiftModifier)))
        return false;

    if (!m_assignments.contains(key) || event->modifiers().testFlag(Qt::ShiftModifier)) {
        const bool assigned = assingKey(key);
        if (!assigned)
            return false;
    }
    const auto match = m_assignments.value(key);

    Q_EMIT requestToggleCategory(match.category, match.value);

    return true;
}

bool AnnotationHandler::assingKey(const QString &key)
{
    SelectCategoryAndValue dialog(i18n("Assign Macro"), i18n("Select item for macro key <b>%1</b>", key));
    auto result = dialog.exec();
    if (result == QDialog::Rejected)
        return false;

    m_assignments[key] = { dialog.category(), dialog.value() };
    saveSettings();
    return true;
}

void AnnotationHandler::saveSettings()
{
    KConfigGroup group = KSharedConfig::openConfig(QString::fromLatin1("kphotoalbumrc"))->group("viewer keybindings");
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
    KConfigGroup group = KSharedConfig::openConfig(QString::fromLatin1("kphotoalbumrc"))->group("viewer keybindings");
    const QStringList keys = group.groupList();
    for (const QString &key : keys) {
        auto subgroup = group.group(key);
        m_assignments[key] = { subgroup.readEntry("category"), subgroup.readEntry("value") };
    }
}

bool AnnotationHandler::askForTagAndInsert()
{
    SelectCategoryAndValue dialog(i18n("Tag Item"), i18n("Select tag for image"));
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

} // namespace Viewer
