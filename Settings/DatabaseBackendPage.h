// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DATABASEBACKENDPAGE_H
#define DATABASEBACKENDPAGE_H

#include <QWidget>

class QCheckBox;
class QSpinBox;

namespace Settings
{
class SettingsData;

class DatabaseBackendPage : public QWidget
{
    Q_OBJECT
public:
    explicit DatabaseBackendPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private Q_SLOTS:
    void markDirty();

private:
    QSpinBox *m_autosave;
    QSpinBox *m_backupCount;
    QCheckBox *m_compressBackup;
    QCheckBox *m_compressedIndexXML;
};

}

#endif /* DATABASEBACKENDPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
