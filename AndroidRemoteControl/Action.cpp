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

void Action::setCurrentPage(const QString& page)
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
    setCurrentPage(QStringLiteral("Overview"));
}

ShowCategoryValueAction::ShowCategoryValueAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowCategoryValueAction::execute()
{
    sendCommand(SearchCommand(SearchType::CategoryItems, m_searchInfo));
    clearCategoryModel();
    setCurrentPage(QStringLiteral("CategoryItems"));
}

ShowThumbnailsAction::ShowThumbnailsAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowThumbnailsAction::execute()
{
    sendCommand(SearchCommand(SearchType::Images, m_searchInfo));
    clearThumbnailsModel();
    setCurrentPage(QString::fromUtf8("Thumbnails"));
}

ShowImagesAction::ShowImagesAction(int imageId, const SearchInfo& searchInfo)
    :Action(searchInfo), m_imageId(imageId)
{
}

void ShowImagesAction::execute()
{
    setCurrentPage(QStringLiteral("ImageViewer"));
    RemoteInterface::instance().setCurrentView(m_imageId);
}


} // namespace RemoteControl
