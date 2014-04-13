#include "History.h"

namespace RemoteControl {

void History::push(std::unique_ptr<Action> action)
{
    m_backward.push(std::move(m_current));
    m_forward = {};
    m_current = std::move(action);
    m_current->run();
}

void History::goForward()
{
    m_backward.push(std::move(m_current));
    m_current = std::move(m_forward.top());
    m_forward.pop();
    m_current->run();
}

void History::goBackward()
{
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

} // namespace RemoteControl
