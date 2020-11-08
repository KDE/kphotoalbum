/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
  
   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAINWINDOW_DUPLICATEMATCH_H
#define MAINWINDOW_DUPLICATEMATCH_H

#include <DB/FileNameList.h>
#include <ImageManager/ImageClientInterface.h>
#include <Utilities/DeleteFiles.h>

#include <QList>
#include <QWidget>

class QLabel;
class QCheckBox;
class QRadioButton;

namespace MainWindow
{

class MergeToolTip;

class DuplicateMatch : public QWidget, ImageManager::ImageClientInterface
{
    Q_OBJECT

public:
    explicit DuplicateMatch(const DB::FileNameList &files);
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    void setSelected(bool);
    bool selected() const;
    void execute(Utilities::DeleteMethod);
    bool eventFilter(QObject *, QEvent *) override;

signals:
    void selectionChanged();

private:
    QLabel *m_image;
    QCheckBox *m_merge;
    QList<QRadioButton *> m_buttons;
};

} // namespace MainWindow

#endif // MAINWINDOW_DUPLICATEMATCH_H
// vi:expandtab:tabstop=4 shiftwidth=4:
