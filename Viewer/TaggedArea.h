/* SPDX-FileCopyrightText: 2014-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef TAGGEDAREA_H
#define TAGGEDAREA_H

#include <QFrame>

namespace Viewer
{

/**
 * @brief The TaggedArea class represents a positionable tag in the viewer.
 * It does not allow any manipulation of the tag data.
 *
 * ## Styling
 * The appearance is based on the properties selected() and highlighted().
 * The following styles are expected to be set for a proper appearance of TaggedArea:
 *  - `Viewer--TaggedArea`
 *  - `Viewer--TaggedArea:hover`
 *  - `Viewer--TaggedArea[highlighted="true"]`
 */
class TaggedArea : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool selected MEMBER m_selected READ selected WRITE setSelected RESET deselect)
    Q_PROPERTY(bool highlighted MEMBER m_highlighted READ highlighted WRITE setHighlighted)

public:
    explicit TaggedArea(QWidget *parent = nullptr);
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
    void checkIsSelected(const QPair<QString, QString> &tagData);

protected:
    /**
     * @brief repolish tells the widget to reevaluate its style.
     * This required when the style is dynamically changed because a property changed.
     */
    void repolish();

private:
    QPair<QString, QString> m_tagInfo;
    QRect m_actualGeometry;
    bool m_selected = false;
    bool m_highlighted = false;
};

}

#endif // TAGGEDAREA_H
// vi:expandtab:tabstop=4 shiftwidth=4:
