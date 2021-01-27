// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SEARCHBAR_H
#define SEARCHBAR_H
#include <QEvent>
#include <ktoolbar.h>
class QLineEdit;
class KMainWindow;

namespace MainWindow
{

class SearchBar : public KToolBar
{
    Q_OBJECT

public:
    explicit SearchBar(KMainWindow *parent);

protected:
    bool eventFilter(QObject *watched, QEvent *e) override;

public slots:
    void reset();
    void setLineEditEnabled(bool b);

signals:
    void textChanged(const QString &);
    void returnPressed();
    void cleared();
    void keyPressed(QKeyEvent *);

private:
    QLineEdit *m_edit;
};
}

#endif /* SEARCHBAR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
