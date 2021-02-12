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

/**
 * @brief The SearchBar class is a thin wrapper around a search box (that is a QLineEdit).
 * It makes the search box usable as a toolbar.
 *
 * It also installs an event filter for the search box that clears the search text when the escape key is pressed,
 * and exposes movement keys via signal.
 *
 * ## Signals:
 *
 * The textChanged() and returnPressed() signals are what you would expect from a QLineEdit.
 * Additionally, there is cleared() signal that is emitted when the SearchBar is reset,
 * and keyPressed() that is emitted for a a few special keys.
 */
class SearchBar : public KToolBar
{
    Q_OBJECT

public:
    explicit SearchBar(KMainWindow *parent);

protected:
    bool eventFilter(QObject *watched, QEvent *e) override;

public slots:
    /**
     * @brief Clears the content of the search box.
     */
    void reset();
    /**
     * @brief setLineEditEnabled calls setEnabled() on the search box.
     * @param enabled
     */
    void setLineEditEnabled(bool enabled);

signals:
    /**
     * @see QLineEdit::textChanged
     */
    void textChanged(const QString &);
    /**
     * @see QLineEdit::returnPressed
     */
    void returnPressed();
    /**
     * @brief cleared is emitted whenever the search box contents are cleared.
     * This can happen either via key press (Escape key) or programatically.
     */
    void cleared();
    /**
     * @brief keyPressed is emitted when a movement key is pressed.
     * QKeyEvents that are signalled this way are:
     *  - arrow keys
     *  - Page up and Page down keys
     *  - Home and End keys
     */
    void keyPressed(QKeyEvent *);

private:
    QLineEdit *m_edit;
};
}

#endif /* SEARCHBAR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
