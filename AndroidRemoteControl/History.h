/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
