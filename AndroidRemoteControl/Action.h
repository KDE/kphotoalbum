/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONTROL_ACTION_H
#define REMOTECONTROL_ACTION_H

#include "../RemoteControl/RemoteCommand.h"
#include "../RemoteControl/Types.h"
#include "DiscoveryModel.h"
#include <QString>

namespace RemoteControl
{

class Action
{
public:
    Action(const SearchInfo &searchInfo);
    virtual ~Action() { }
    void run();
    virtual void save() {};

protected:
    virtual void execute() = 0;
    void setCurrentPage(Page page);
    void sendCommand(const RemoteCommand &command);
    void clearCategoryModel();
    SearchInfo m_searchInfo;
};

class ShowOverviewAction : public Action
{
public:
    ShowOverviewAction(const SearchInfo &searchInfo);

protected:
    void execute() override;
};

class ShowCategoryValueAction : public Action
{
public:
    ShowCategoryValueAction(const SearchInfo &searchInfo, CategoryViewType type);

protected:
    void execute() override;
    void save() override;

private:
    CategoryViewType m_type;
    int m_index = 0;
};

class ShowThumbnailsAction : public Action
{
public:
    ShowThumbnailsAction(const SearchInfo &searchInfo, int imageId = -1);

protected:
    void execute() override;
    void save() override;

private:
    int m_scrolledToIndex = -1;
    int m_initialImageRequest = -1;
};

class ShowImagesAction : public Action
{
public:
    ShowImagesAction(int imageId, const SearchInfo &searchInfo);

protected:
    void execute() override;

private:
    const int m_imageId;
};

class DiscoverAction : public Action
{
public:
    DiscoverAction(const SearchInfo &searchInfo, DiscoveryModel *model);
    void setCurrentSelection(const QList<int> &selection, const QList<int> &allImages);

protected:
    void execute() override;
    QList<int> m_currentSelection;
    QList<int> m_allImages;
    DiscoveryModel *m_model;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_ACTION_H
