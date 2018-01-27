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

#include "History.h"

namespace RemoteControl {

void History::push(std::unique_ptr<Action> action)
{
    if (m_current)
        m_current->save();
    m_backward.push(std::move(m_current));
    m_forward = std::stack<std::unique_ptr<Action>>();
    m_current = std::move(action);
    m_current->run();
}

void History::goForward()
{
    if (m_current)
        m_current->save();
    m_backward.push(std::move(m_current));
    m_current = std::move(m_forward.top());
    m_forward.pop();
    m_current->run();
}

void History::goBackward()
{
    if (m_current)
        m_current->save();
    m_forward.push(std::move(m_current));
    m_current = std::move(m_backward.top());
    m_backward.pop();
    m_current->run();
}

bool History::canGoBack() const
{
    return m_backward.size() > 1;
}

bool History::canGoForward() const
{
    return !m_forward.empty();
}

void History::rerunTopItem()
{
    m_current->run();
}

} // namespace RemoteControl
