/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BREADCRUMB_H
#define BREADCRUMB_H
#include <QString>

namespace Browser
{

/**
 * \brief Information about a single breadcrumb
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This is basically a simple class to make the code for handling
 * breadcrumbs simpler. It encodes the following information about a
 * breadcrumb:
 * \li Is this a first breadcrumb (the result of going home e.g.)
 * \li which text should be shown for this breadcrumb.
 *
 */
class Breadcrumb
{
public:
    static Breadcrumb empty();
    static Breadcrumb home();
    static Breadcrumb view();

    explicit Breadcrumb(const QString &text, bool isBeginning = false);
    QString text() const;
    bool isBeginning() const;
    bool isView() const;
    bool operator==(const Breadcrumb &other) const;
    bool operator!=(const Breadcrumb &other) const;

private:
    int m_index;
    bool m_isBeginning;
    bool m_isView;
    QString m_text;
    static int s_count;
};

}

#endif /* BREADCRUMB_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
