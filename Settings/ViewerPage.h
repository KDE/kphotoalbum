/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef VIEWERPAGE_H
#define VIEWERPAGE_H
#include <QWidget>
#include <kpabase/SettingsData.h>

class KComboBox;
class QSpinBox;
class QComboBox;
class QPushButton;

namespace Settings
{
class SettingsData;
class ViewerSizeConfig;

class ViewerPage : public QWidget
{
public:
    explicit ViewerPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);

private:
    Settings::ViewerSizeConfig *m_slideShowSetup;
    Settings::ViewerSizeConfig *m_viewImageSetup;
    QComboBox *m_smoothScale;
    QSpinBox *m_slideShowInterval;
    QSpinBox *m_cacheSize;
    KComboBox *m_viewerStandardSize;
    QPushButton *m_videoBackendButton;
    Settings::VideoBackend m_videoBackend = Settings::VideoBackend::NotConfigured;
};

}

#endif /* VIEWERPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
