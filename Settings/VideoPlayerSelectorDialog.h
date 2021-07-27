// SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>
#include <kpabase/SettingsData.h>

class QRadioButton;

namespace Settings
{

class VideoPlayerSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    VideoPlayerSelectorDialog(QWidget *parent = nullptr);
    Settings::VideoBackend backend() const;

private:
    QRadioButton *m_vlc;
    QRadioButton *m_qtav;
    QRadioButton *m_phonon;
};

} // namespace Settings
