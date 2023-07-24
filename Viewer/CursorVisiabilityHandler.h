// SPDX-FileCopyrightText: 2023 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QStack>
#include <QWidget>

/**
  This class handles hiding the mouse cursor when it is not needed while viewing images or videos.

  In some situations, e.g. when selecting an area for zooming, or when bringing up the annotation dialog,
  this handling is temporarily disabled.
  */
class CursorVisiabilityHandler : public QObject
{
    Q_OBJECT
public:
    explicit CursorVisiabilityHandler(QWidget *parentWidget);
    void disableCursorHiding();
    void enableCursorHiding();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void showCursorTemporarily();
    void hideCursor();

    QWidget *m_parentWidget;
    QTimer *m_timer;
    QStack<bool> m_cursorHidingEnabled;
};
