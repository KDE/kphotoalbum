// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXERNALTOOLSPAGE_H
#define EXERNALTOOLSPAGE_H

#include <KUrlRequester>
#include <QWidget>

namespace Settings
{
class SettingsData;

class ExternalToolsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ExternalToolsPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private:
    KUrlRequester *m_imageToolRequester;
    KUrlRequester *m_videoToolRequester;
};

}

#endif /* EXERNALTOOLSPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
