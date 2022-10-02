// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGECOUNTER_H
#define IMAGECOUNTER_H
#include <qlabel.h>

namespace MainWindow
{

class ImageCounter : public QLabel
{
    Q_OBJECT

public:
    explicit ImageCounter(QWidget *parent);

public Q_SLOTS:
    void setMatchCount(int matches);
    void setSelectionCount(int selected);
    void setTotal(int);
    void showBrowserMatches(int matches);
};
}

#endif /* IMAGECOUNTER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
