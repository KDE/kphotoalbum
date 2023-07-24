// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "AnnotationHandler.h"
#include <QDialog>

class KActionCollection;

namespace Viewer
{

namespace Ui
{
    class AnnotationHelp;
}

class AnnotationHelp : public QDialog
{
    Q_OBJECT

public:
    explicit AnnotationHelp(KActionCollection *actions, const AnnotationHandler::Assignments &assignments, QWidget *parent = nullptr);
    ~AnnotationHelp();

private:
    Ui::AnnotationHelp *ui;
};

} // namespace Viewer
