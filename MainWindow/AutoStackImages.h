/* Copyright (C) 2010 Miika Turkia <miika.turkia@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef AUTOSTACKIMAGES_H
#define AUTOSTACKIMAGES_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QRadioButton;

namespace DB
{
class FileNameList;
}

namespace MainWindow
{

class AutoStackImages : public QDialog
{
    Q_OBJECT

public:
    AutoStackImages(QWidget *parent, const DB::FileNameList &list);

protected slots:
    void accept() override;

private:
    QCheckBox *m_matchingMD5;
    QCheckBox *m_matchingFile;
    QCheckBox *m_origTop;
    QCheckBox *m_continuousShooting;
    QRadioButton *m_autostackUnstack;
    QRadioButton *m_autostackSkip;
    QRadioButton *m_autostackDefault;
    QSpinBox *m_continuousThreshold;
    const DB::FileNameList &m_list;
    virtual void matchingMD5(DB::FileNameList &toBeShown);
    virtual void matchingFile(DB::FileNameList &toBeShown);
    virtual void continuousShooting(DB::FileNameList &toBeShown);
};
}

#endif /* AUTOSTACKIMAGES_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
