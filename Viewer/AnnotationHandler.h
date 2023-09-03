// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KLocalizedString>
#include <QMap>
#include <QObject>

class QKeyEvent;

namespace Viewer
{

class AnnotationHandler : public QObject
{
    Q_OBJECT

public:
    AnnotationHandler(QObject *parent);
    bool handle(QKeyEvent *event);
    bool askForTagAndInsert();

    struct Assignment {
        QString category;
        QString value;
    };
    using Key = QString;
    using Assignments = QMap<Key, Assignment>;

    Assignments assignments() const;

Q_SIGNALS:
    void requestToggleCategory(const QString &category, const QString &value);
    void requestHelp();

private:
    bool assignKey(const QString &key);
    void clearKey(const QString &key);
    void saveSettings();
    void loadSettings();

    Assignments m_assignments;
};

QDebug operator<<(QDebug debug, const AnnotationHandler::Assignment &a);

} // namespace Viewer
