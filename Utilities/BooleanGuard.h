/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BOOLEANGUARD_H
#define BOOLEANGUARD_H

namespace Utilities
{

class BooleanGuard
{
public:
    explicit BooleanGuard(bool &guard);
    ~BooleanGuard();
    bool canContinue();

private:
    bool &m_guard;
    bool m_iLockedIt;
};

}

#endif /* BOOLEANGUARD_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
