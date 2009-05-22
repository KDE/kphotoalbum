#include "BrowserAction.h"

Browser::BrowserAction::BrowserAction( BrowserWidget* browser )
     :_browser(browser)
{
}

Browser::BrowserWidget* Browser::BrowserAction::browser() const
{
    return _browser;
}

Browser::BrowserAction* Browser::BrowserAction::generateChildAction( const QModelIndex &)
{
    return 0;
}

Browser::Viewer Browser::BrowserAction::viewer()
{
    return ShowBrowser;
}

