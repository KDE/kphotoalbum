// SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>
#include <kpabase/SettingsData.h>

class QRadioButton;

namespace Settings
{

/**
 * @brief availableVideoBackends shows what video backends were enabled during compilation.
 * @return the available video backends
 */
constexpr VideoBackends availableVideoBackends();

/**
 * @brief preferredVideoBackend checks the configuredBackend against available video backends and an optional exclusions list.
 * The hard coded ranking of video backends was empirically determined and may change in the future.
 * @return the best suited VideoBackend, or VideoBackend::NotConfigured if none is viable.
 */
Settings::VideoBackend preferredVideoBackend(const Settings::VideoBackend configuredBackend, const Settings::VideoBackends exclusions = {});

/**
 * @brief localizedEnumName returns a localized value for the enum, for use in localized messages.
 * @param backend the VideoBackend
 * @return a localized friendly name representing the enum value
 */
QString localizedEnumName(const Settings::VideoBackend backend);

class VideoPlayerSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    VideoPlayerSelectorDialog(QWidget *parent = nullptr);
    Settings::VideoBackend backend() const;

private:
    QRadioButton *m_vlc;
    QRadioButton *m_phonon;
};

} // namespace Settings
