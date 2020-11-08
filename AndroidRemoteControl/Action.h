/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef REMOTECONTROL_ACTION_H
#define REMOTECONTROL_ACTION_H

#include "DiscoveryModel.h"
#include "RemoteCommand.h"
#include "Types.h"
#include <QString>

namespace RemoteControl
{

class Action
{
public:
    Action(const SearchInfo &searchInfo);
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
    ShowThumbnailsAction(const SearchInfo &searchInfo);

protected:
    void execute() override;
    void save() override;

private:
    int m_index = 0;
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
