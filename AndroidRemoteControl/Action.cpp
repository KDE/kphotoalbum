#include "Action.h"
#include "RemoteInterface.h"

namespace RemoteControl {


Action::Action(const SearchInfo& searchInfo)
    :m_searchInfo(searchInfo)
{
}

void Action::run()
{
    execute();
    RemoteInterface::instance().m_search = m_searchInfo;
}

void Action::setCurrentPage(Page page)
{
    RemoteInterface::instance().setCurrentPage(page);
}

void Action::sendCommand(const RemoteCommand& command)
{
    RemoteInterface::instance().sendCommand(command);
}

void Action::clearCategoryModel()
{
    RemoteInterface::instance().m_categoryItems->setImages({});
}

void Action::clearThumbnailsModel()
{
    RemoteInterface::instance().m_thumbnailModel->setImages({});
}




ShowOverviewAction::ShowOverviewAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowOverviewAction::execute()
{
    sendCommand(SearchCommand(SearchType::Categories, m_searchInfo));
    setCurrentPage(Page::OverviewPage);
}

ShowCategoryValueAction::ShowCategoryValueAction(const SearchInfo& searchInfo, CategoryViewType type)
    :Action(searchInfo), m_type(type)
{
}

void ShowCategoryValueAction::execute()
{
    sendCommand(SearchCommand(SearchType::CategoryItems, m_searchInfo));
    clearCategoryModel();
    if (m_type == CategoryViewType::CategoryIconView)
        setCurrentPage(Page::CategoryItemsPage);
    else
        setCurrentPage(Page::CategoryListPage);
}

ShowThumbnailsAction::ShowThumbnailsAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowThumbnailsAction::execute()
{
    sendCommand(SearchCommand(SearchType::Images, m_searchInfo));
    clearThumbnailsModel();
    setCurrentPage(Page::ThumbnailsPage);
}

ShowImagesAction::ShowImagesAction(int imageId, const SearchInfo& searchInfo)
    :Action(searchInfo), m_imageId(imageId)
{
}

void ShowImagesAction::execute()
{
    setCurrentPage(Page::ImageViewerPage);
    RemoteInterface::instance().setCurrentView(m_imageId);
}


} // namespace RemoteControl
