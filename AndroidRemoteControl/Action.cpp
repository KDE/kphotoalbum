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

void Action::setCurrentPage(Types::Page page)
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
    setCurrentPage(Types::Page::Overview);
}

ShowCategoryValueAction::ShowCategoryValueAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowCategoryValueAction::execute()
{
    sendCommand(SearchCommand(SearchType::CategoryItems, m_searchInfo));
    clearCategoryModel();
    setCurrentPage(Types::Page::CategoryItems);
}

ShowThumbnailsAction::ShowThumbnailsAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowThumbnailsAction::execute()
{
    sendCommand(SearchCommand(SearchType::Images, m_searchInfo));
    clearThumbnailsModel();
    setCurrentPage(Types::Page::Thumbnails);
}

ShowImagesAction::ShowImagesAction(int imageId, const SearchInfo& searchInfo)
    :Action(searchInfo), m_imageId(imageId)
{
}

void ShowImagesAction::execute()
{
    setCurrentPage(Types::Page::ImageViewer);
    RemoteInterface::instance().setCurrentView(m_imageId);
}


} // namespace RemoteControl
