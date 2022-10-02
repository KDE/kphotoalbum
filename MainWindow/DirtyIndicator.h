// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DIRTYINDICATOR_H
#define DIRTYINDICATOR_H

#include <qlabel.h>
#include <qpixmap.h>

namespace MainWindow
{
class Window;

class DirtyIndicator : public QLabel
{
    Q_OBJECT

public:
    static void markDirty();
    static void suppressMarkDirty(bool state);

public Q_SLOTS:
    void markDirtySlot();

Q_SIGNALS:
    void dirty();

private:
    friend class StatusBar;
    friend class Window;
    DirtyIndicator(QWidget *parent);
    void autoSaved();
    void saved();
    bool isSaveDirty() const;
    bool isAutoSaveDirty() const;

    QPixmap m_dirtyPix;
    static bool s_autoSaveDirty;
    static bool s_saveDirty;

    static bool s_suppressMarkDirty;
};

}

#endif /* DIRTYINDICATOR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
