// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef BACKGROUNDTASKS_JOBVIEWER_H
#define BACKGROUNDTASKS_JOBVIEWER_H

#include <QDialog>

class QTreeView;
class QPushButton;

namespace BackgroundTaskManager
{

class JobModel;

class JobViewer : public QDialog
{
    Q_OBJECT

public:
    explicit JobViewer(QWidget *parent = nullptr);
    void setVisible(bool) override;

private Q_SLOTS:
    void togglePause();

private:
    void updatePauseButton();
    JobModel *m_model;
    QTreeView *m_treeView;
    QPushButton *m_pauseButton;
};

} // namespace BackgroundTaskManager

#endif // BACKGROUNDTASKS_JOBVIEWER_H

// vi:expandtab:tabstop=4 shiftwidth=4:
