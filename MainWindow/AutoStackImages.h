/* SPDX-FileCopyrightText: 2010 Miika Turkia <miika.turkia@gmail.com>

   SPDX-License-Identifier: GPL-2.0-or-later
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
