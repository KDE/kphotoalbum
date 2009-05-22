#include "BrowserAction.h"

Browser::BrowserAction::BrowserAction( BrowserWidget* browser )
     :_browser(browser)
{
}

Browser::BrowserWidget* Browser::BrowserAction::browser() const
{
    return _browser;
}

