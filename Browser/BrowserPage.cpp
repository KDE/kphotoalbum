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
 * Return a page for the item at the given model index. In case the
 * activation doesn't result in a new page, simply return 0 (This is for
 * example the case if a search is executed, but canceled).
 */
Browser::BrowserPage* Browser::BrowserPage::activateChild( const QModelIndex &)
{
    return 0;
}

Browser::Viewer Browser::BrowserPage::viewer()
{
    return ShowBrowser;
}

/**
 * When this page is active, should the search bar in the browser be
 * enabled?
 */
bool Browser::BrowserPage::isSearchable() const
{
    return true;
}

/**
 * \return whether the user should be allowed to change the view type (tree
 * view vs iconview
 */
bool Browser::BrowserPage::isViewChangeable() const
{
    return isSearchable();
}

/**
 * \return the viewtype which should be used in case this is to be shown in
 * the Browser.
 */
DB::Category::ViewType Browser::BrowserPage::viewType() const
{
    return DB::Category::IconView;
}

/**
 * \return the \ref DB::ImageSearchInfo this item was constructed with.
 */
DB::ImageSearchInfo Browser::BrowserPage::searchInfo() const
{
    return _info;
}

/**
 * \return the breadcrumb to be used in the status bar for this item
 */
Browser::Breadcrumb Browser::BrowserPage::breadcrumb() const
{
    return Breadcrumb::empty();
}

/**
 * \return if this page should be shown when moving backward/forward
 * through the history of pages.
 */
bool Browser::BrowserPage::showDuringMovement() const
{
    return false;
}
