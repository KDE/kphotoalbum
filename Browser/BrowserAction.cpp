#include "BrowserAction.h"

Browser::BrowserAction::BrowserAction( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : _info(info), _browser(browser)
{
}

/**
 * \return the associated \ref BrowserWidget. This instance is needed when
 * the action is creating new actions for a child aciton.
 */
Browser::BrowserWidget* Browser::BrowserAction::browser() const
{
    return _browser;
}

/**
 * Return an action for the item at the given model index
 */
Browser::BrowserAction* Browser::BrowserAction::generateChildAction( const QModelIndex &)
{
    return 0;
}

Browser::Viewer Browser::BrowserAction::viewer()
{
    return ShowBrowser;
}

bool Browser::BrowserAction::isSearchable() const
{
    return true;
}

bool Browser::BrowserAction::isViewChangeable() const
{
    return isSearchable();
}

DB::Category::ViewType Browser::BrowserAction::viewType() const
{
    return DB::Category::IconView;
}

DB::ImageSearchInfo Browser::BrowserAction::searchInfo() const
{
    return _info;
}

Browser::Breadcrumb Browser::BrowserAction::breadcrumb() const
{
    return Breadcrumb::empty();
}

bool Browser::BrowserAction::showDuringBack() const
{
    return false;
}
