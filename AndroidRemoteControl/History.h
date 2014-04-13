#ifndef REMOTECONTROL_HISTORY_H
#define REMOTECONTROL_HISTORY_H

#include <QStack>
#include "Action.h"
#include <memory>
#include <stack>

namespace RemoteControl {

class History
{
public:
    void push(std::unique_ptr<Action>);
    void goForward();
    void goBackward();
    bool canGoBack() const;
    bool canGoForward() const;

private:
    std::stack<std::unique_ptr<Action>> m_backward;
    std::stack<std::unique_ptr<Action>> m_forward;
    std::unique_ptr<Action> m_current = nullptr;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_HISTORY_H
