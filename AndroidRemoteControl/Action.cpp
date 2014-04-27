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
    RemoteInterface::instance().setActiveThumbnailModel(RemoteInterface::ModelType::Thumbnail);
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

DiscoverAction::DiscoverAction(const SearchInfo& searchInfo)
 : Action(searchInfo)
{
}

void DiscoverAction::execute()
{
    setCurrentPage(Page::DiscoverPage);
    RemoteInterface::instance().setActiveThumbnailModel(RemoteInterface::ModelType::Discovery);
    sendCommand(SearchCommand(SearchType::Images, m_searchInfo));
}

void ShowDiscoveredImage::execute()
{
    setCurrentPage(Page::ImageViewerPage);
    // FIXME: We should do this from action to limit friendship
    // besides, I wonderif messing with m_thumbnailModel is the right thing
    RemoteInterface::instance().m_thumbnailModel->setImages({m_imageId});
}


} // namespace RemoteControl
