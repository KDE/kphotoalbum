/* Copyright (C) 2014-2019 The KPhotoAlbum Development Team

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
#ifndef TAGGEDAREA_H
#define TAGGEDAREA_H

#include <QFrame>

namespace Viewer
{

class TaggedArea : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool selected MEMBER m_selected READ selected WRITE setSelected RESET deselect)
    Q_PROPERTY(bool highlighted MEMBER m_highlighted READ highlighted WRITE setHighlighted)

public:
    explicit TaggedArea(QWidget *parent = 0);
    ~TaggedArea() override;
    void setTagInfo(QString category, QString localizedCategory, QString tag);
    void setActualGeometry(QRect geometry);
    QRect actualGeometry() const;

    /**
     * @brief When selected, the TaggedArea is shown (just like when hovering with the mouse).
     * This is used to make the area visible when the corresponding tag in the ViewerWidget is hovered.
     * @return \c true, if the area is visible.
     */
    bool selected() const;
    void setSelected(bool selected);
    void deselect();

    /**
     * @brief highlighted
     * @return \c true, when the area should be visibly highlighted, \c false otherwise.
     */
    bool highlighted() const;
    /**
     * @brief setHighlighted sets the highlighted property of the area.
     * An area with the highlighted tag set to \c true will be visibly highlighted.
     * @param highlighted
     */
    void setHighlighted(bool highlighted);

public slots:
    /**
     * @brief checkIsSelected set the \c selected property if tagData matches the tag.
     * @param tagData
     */
    void checkIsSelected(QPair<QString, QString> tagData);

private:
    QPair<QString, QString> m_tagInfo;
    QRect m_actualGeometry;
    bool m_selected;
    bool m_highlighted;
};

}

#endif // TAGGEDAREA_H
// vi:expandtab:tabstop=4 shiftwidth=4:
