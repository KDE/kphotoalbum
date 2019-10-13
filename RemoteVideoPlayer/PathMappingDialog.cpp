#include "PathMappingDialog.h"
#include "ui_PathMappingDialog.h"

#include <QFileDialog>

PathMappingDialog::PathMappingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PathMappingDialog)
{
    ui->setupUi(this);
    connect(ui->edit, &QToolButton::clicked, this, &PathMappingDialog::configureHostPath);
}

PathMappingDialog::~PathMappingDialog()
{
    delete ui;
}

QString PathMappingDialog::linuxPath() const
{
    return ui->linuxPath->text();
}

QString PathMappingDialog::hostPath() const
{
    return ui->hostPath->text();
}

void PathMappingDialog::setLinuxPath(const QString &path)
{
    ui->linuxPath->setText(path);
}

void PathMappingDialog::setHostPath(const QString &path)
{
    ui->hostPath->setText(path);
}

void PathMappingDialog::configureHostPath()
{
    const QString path = QFileDialog::getExistingDirectory(this, {}, ui->hostPath->text());
    if (!path.isNull())
        ui->hostPath->setText(path);
}
