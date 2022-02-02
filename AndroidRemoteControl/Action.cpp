/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Action.h"
#include "PositionObserver.h"
#include "RemoteInterface.h"
#include "ScreenInfo.h"
extern QQuickView *view;

namespace RemoteControl
{

Action::Action(const SearchInfo &searchInfo)
    : m_searchInfo(searchInfo)
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

void Action::sendCommand(const RemoteCommand &command)
{
    RemoteInterface::instance().sendCommand(command);
}

void Action::clearCategoryModel()
{
    RemoteInterface::instance().m_categoryItems->setImages({});
}

ShowOverviewAction::ShowOverviewAction(const SearchInfo &searchInfo)
    : Action(searchInfo)
{
}

void ShowOverviewAction::execute()
{
    int size = ScreenInfo::instance().overviewIconSize();
    sendCommand(SearchRequest(SearchType::Categories, m_searchInfo, size));
    setCurrentPage(Page::OverviewPage);
}

ShowCategoryValueAction::ShowCategoryValueAction(const SearchInfo &searchInfo, CategoryViewType type)
    : Action(searchInfo)
    , m_type(type)
{
}

void ShowCategoryValueAction::execute()
{
    sendCommand(SearchRequest(SearchType::CategoryItems, m_searchInfo));
    clearCategoryModel();
    if (m_type == CategoryViewType::CategoryIconView) {
        PositionObserver::setCategoryIconViewOffset(m_index);
        setCurrentPage(Page::CategoryItemsPage);
    } else {
        PositionObserver::setCategoryListViewOffset(m_index);
        setCurrentPage(Page::CategoryListPage);
    }
}

void ShowCategoryValueAction::save()
{
    if (m_type == CategoryViewType::CategoryIconView)
        m_index = PositionObserver::categoryIconViewOffset();
    else
        m_index = PositionObserver::categoryListViewOffset();
}

ShowThumbnailsAction::ShowThumbnailsAction(const SearchInfo &searchInfo, int imageId)
    : Action(searchInfo)
    , m_initialImageRequest(imageId)
{
}

void ShowThumbnailsAction::execute()
{
    SearchRequest request(SearchType::Images, m_searchInfo);
    if (m_scrolledToIndex == -1) {
        // We need to send the request to the server and back, as it is only when we have the
        // request back that we can look up the index of the images
        request.focusImage = m_initialImageRequest;
    }

    sendCommand(request);
    RemoteInterface::instance().setActiveThumbnailModel(RemoteInterface::ModelType::Thumbnail);
    setCurrentPage(Page::ThumbnailsPage);

    if (m_scrolledToIndex != -1)
        PositionObserver::setThumbnailOffset(m_scrolledToIndex);
}

void ShowThumbnailsAction::save()
{
    m_scrolledToIndex = PositionObserver::thumbnailOffset();
}

ShowImagesAction::ShowImagesAction(int imageId, const SearchInfo &searchInfo)
    : Action(searchInfo)
    , m_imageId(imageId)
{
}

void ShowImagesAction::execute()
{
    setCurrentPage(Page::ImageViewerPage);
    RemoteInterface::instance().setCurrentView(m_imageId);
}

DiscoverAction::DiscoverAction(const SearchInfo &searchInfo, DiscoveryModel *model)
    : Action(searchInfo)
    , m_model(model)
{
}

void DiscoverAction::setCurrentSelection(const QList<int> &selection, const QList<int> &allImages)
{
    m_currentSelection = selection;
    m_allImages = allImages;
}

void DiscoverAction::execute()
{
    RemoteInterface::instance().setActiveThumbnailModel(RemoteInterface::ModelType::Discovery);

    if (m_currentSelection.isEmpty())
        sendCommand(SearchRequest(SearchType::Images, m_searchInfo));
    else
        m_model->setCurrentSelection(m_currentSelection, m_allImages);

    m_model->setCurrentAction(this);
    setCurrentPage(Page::DiscoverPage);
}

} // namespace RemoteControl
