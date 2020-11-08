/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INVALIDDATEFINDER_H
#define INVALIDDATEFINDER_H

#include <QDialog>
class QRadioButton;

namespace MainWindow
{

class InvalidDateFinder : public QDialog
{
    Q_OBJECT

public:
    explicit InvalidDateFinder(QWidget *parent);

protected slots:
    void accept() override;

private:
    QRadioButton *m_dateNotTime;
    QRadioButton *m_missingDate;
    QRadioButton *m_partialDate;
};
}

#endif /* INVALIDDATEFINDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
