#ifndef REMOTECONTROL_HISTORY_H
#define REMOTECONTROL_HISTORY_H

#include <QStack>
#include "Action.h"

namespace RemoteControl {

class History
{
public:
    void push(Action*);
    void goForward();
    void goBackward();
    bool canGoBack() const;
    bool canGoForward() const;

private:
    QStack<Action*> m_backward;
    QStack<Action*> m_forward;
    Action* m_current = nullptr;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_HISTORY_H
