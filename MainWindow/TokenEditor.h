/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef TOKENEDITOR_H
#define TOKENEDITOR_H

#include <QDialog>
#include <QList>
class QCheckBox;

namespace MainWindow
{

class TokenEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TokenEditor(QWidget *parent);
    virtual void show();
    static QStringList tokensInUse();

protected slots:
    void selectAll();
    void selectNone();
    void accept() override;

private:
    QList<QCheckBox *> m_checkBoxes;
};
}

#endif /* TOKENEDITOR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
