#ifndef PATHMAPPINGDIALOG_H
#define PATHMAPPINGDIALOG_H

#include <QDialog>

namespace Ui
{
class PathMappingDialog;
}

class PathMappingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PathMappingDialog(QWidget *parent = nullptr);
    ~PathMappingDialog();
    QString linuxPath() const;
    QString hostPath() const;
    void setLinuxPath(const QString &path);
    void setHostPath(const QString &path);

private:
    void configureHostPath();

    Ui::PathMappingDialog *ui;
};

#endif // PATHMAPPINGDIALOG_H
