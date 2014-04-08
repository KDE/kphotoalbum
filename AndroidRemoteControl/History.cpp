#include "History.h"

namespace RemoteControl {

void History::push(Action* action)
{
    m_backward.push(m_current);
    m_forward.clear();
    m_current = action;
    action->run();
}

void History::goForward()
{
    m_backward.push(m_current);
    m_current = m_forward.pop();
    m_current->run();
}

void History::goBackward()
{
    m_forward.push(m_current);
    m_current = m_backward.pop();
    m_current->run();
}

bool History::canGoBack() const
{
    return m_backward.count() > 1;
}

bool History::canGoForward() const
{
    return !m_forward.isEmpty();
}

} // namespace RemoteControl
