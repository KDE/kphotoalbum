// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "BreadcrumbViewer.h"

#include <QStatusBar>

class QToolButton;
class QTimer;
class QProgressBar;
class QLabel;
class QSlider;

namespace MainWindow
{
class ImageCounter;
class DirtyIndicator;

class StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    StatusBar();
    DirtyIndicator *mp_dirtyIndicator;
    ImageCounter *mp_partial;
    ImageCounter *mp_selected;
    BreadcrumbViewer *mp_pathIndicator;

    void setLocked(bool locked);
    void startProgress(const QString &text, int total);
    void setProgress(int progress);
    void setProgressBarVisible(bool);

    void showThumbnailSlider();
    void hideThumbnailSlider();

Q_SIGNALS:
    void cancelRequest();
    void thumbnailSettingsRequested();

protected:
    void enterEvent(QEvent *event) override;

private Q_SLOTS:
    void hideStatusBar();
    void showStatusBar();

private:
    void setupGUI();
    void setPendingShow();

    QLabel *m_lockedIndicator;
    QProgressBar *m_progressBar;
    QToolButton *m_cancel;
    QTimer *m_pendingShowTimer;
    QSlider *m_thumbnailSizeSlider;
    QToolButton *m_thumbnailSettings;
};

}

#endif /* STATUSBAR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
