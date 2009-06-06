#include "BrowserPage.h"

Browser::BrowserPage::BrowserPage( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : _info(info), _browser(browser)
{
}

/**
 * \return the associated \ref BrowserWidget. This instance is needed when
 * the action is creating new actions for a child aciton.
 */
Browser::BrowserWidget* Browser::BrowserPage::browser() const
{
    return _browser;
}

/**
 * Return an action for the item at the given model index
 */
Browser::BrowserPage* Browser::BrowserPage::activateChild( const QModelIndex &)
{
    return 0;
}

Browser::Viewer Browser::BrowserPage::viewer()
{
    return ShowBrowser;
}

bool Browser::BrowserPage::isSearchable() const
{
    return true;
}

bool Browser::BrowserPage::isViewChangeable() const
{
    return isSearchable();
}

DB::Category::ViewType Browser::BrowserPage::viewType() const
{
    return DB::Category::IconView;
}

DB::ImageSearchInfo Browser::BrowserPage::searchInfo() const
{
    return _info;
}

Browser::Breadcrumb Browser::BrowserPage::breadcrumb() const
{
    return Breadcrumb::empty();
}

bool Browser::BrowserPage::showDuringBack() const
{
    return false;
}
