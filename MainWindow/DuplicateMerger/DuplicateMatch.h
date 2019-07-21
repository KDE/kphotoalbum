/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
