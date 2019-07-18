/* Copyright (C) 2019 The KPhotoAlbum Development Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e. V. (or its successor approved
 *  by the membership of KDE e. V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KPHOTOALBUM_PURPOSEMENU_H
#define KPHOTOALBUM_PURPOSEMENU_H

#include <config-kpa-kipi.h>

#include <QObject>
#include <QString>
#include <QUrl>

class QMenu;

namespace Purpose
{
class Menu;
}

namespace Plugins
{

class PurposeMenu : public QObject
{
    Q_OBJECT
public:
    explicit PurposeMenu(QMenu *parent);

public slots:
    void slotSelectionChanged();

signals:
    /**
     * @brief imageShared is emitted when an image was shared successfully.
     * The url contains the optional location of the shared data
     * (e.g. for plugins that upload to a remote location).
     */
    void imageShared(QUrl);
    void imageSharingFailed(QString message);

private:
    QMenu *m_parentMenu;
    Purpose::Menu *m_purposeMenu;
    bool m_menuUpdateNeeded; ///< Keeps track of changed image selection
    /**
     * @brief Load the Purpose::Menu, add it to the parent menu, and set up connections.
     */
    void loadPurposeMenu();
    /**
     * @brief Load Purpose menu items into the Purpose::Menu.
     * This is dependent on the current set of images.
     */
    void loadPurposeItems();
};

}

#endif /* PURPOSEMENU_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
