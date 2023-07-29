// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AnnotationHelp.h"
#include "ui_AnnotationHelp.h"
#include <KActionCollection>
#include <QStandardItemModel>

namespace Viewer
{

AnnotationHelp::AnnotationHelp(KActionCollection *actions, const AnnotationHandler::Assignments &assignments, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AnnotationHelp)
{
    ui->setupUi(this);

    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList { i18n("Key"), i18n("Tag"), QString(), i18n("Key"), i18n("Tag") });
    int row = 0;
    int column = 0;
    auto addAssignment = [&](const QString &key, const QString &assignment) {
        if (column == 2)
            ++column;
        if (column == 5) {
            ++row;
            column = 0;
        }
        model->setItem(row, column++, new QStandardItem(key));
        model->setItem(row, column++, new QStandardItem(assignment));
    };

    addAssignment(actions->action(QString::fromLatin1("viewer-add-tag"))->shortcut().toString(), i18n("Add Tag"));
    addAssignment(actions->action(QString::fromLatin1("viewer-edit-description"))->shortcut().toString(), i18n("Add Description"));
    addAssignment(actions->action(QString::fromLatin1("viewer-copy-tag-from-previous-image"))->shortcut().toString(), i18n("Copy Annotation from Previous Image"));
    row += 2;
    column = 0;

    for (auto it = assignments.cbegin(); it != assignments.cend(); ++it) {
        const AnnotationHandler::Assignment assignment = it.value();
        addAssignment(it.key(), QLatin1String("%1 / %2").arg(assignment.category, assignment.value));
    }

    ui->assignments->setModel(model);
    ui->assignments->verticalHeader()->hide();
    ui->assignments->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->assignments->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

AnnotationHelp::~AnnotationHelp()
{
    delete ui;
}

} // namespace Viewer
