/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "History.h"

namespace RemoteControl
{

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
