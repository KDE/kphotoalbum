/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef REMOTECONTROL_HISTORY_H
#define REMOTECONTROL_HISTORY_H

#include "Action.h"
#include <QStack>
#include <memory>
#include <stack>

namespace RemoteControl
{

class History
{
public:
    void push(std::unique_ptr<Action>);
    void goForward();
    void goBackward();
    bool canGoBack() const;
    bool canGoForward() const;
    void rerunTopItem();

private:
    std::stack<std::unique_ptr<Action>> m_backward;
    std::stack<std::unique_ptr<Action>> m_forward;
    std::unique_ptr<Action> m_current = nullptr;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_HISTORY_H
