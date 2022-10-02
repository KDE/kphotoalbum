// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include <qdialog.h>

class QLineEdit;

namespace MainWindow
{

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = nullptr);
    QString configFileName() const;

protected Q_SLOTS:
    void slotLoadDemo();
    void createSetup();
    void checkFeatures();

private:
    QString m_configFile;
};

class FileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FileDialog(QWidget *parent);
    QString getFileName();
protected Q_SLOTS:
    void slotBrowseForDirecory();

private:
    QLineEdit *m_lineEdit;
};

}

#endif // WELCOMEDIALOG_H
// vi:expandtab:tabstop=4 shiftwidth=4:
