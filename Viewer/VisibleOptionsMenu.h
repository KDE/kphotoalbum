// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VISIBLEOPTIONSMENU_H
#define VISIBLEOPTIONSMENU_H

#include <QList>
#include <QMenu>

class KToggleAction;
class KActionCollection;

namespace Viewer
{

/**
 * @brief The VisibleOptionsMenu lets the user choose what kind of information is shown in the InfoBox.
 * It is used in the context menu of the viewer.
 *
 * If any option is changed, visibleOptionsChanged() is emitted.
 * Changes are written directly to the global application settings.
 */
class VisibleOptionsMenu : public QMenu
{
    Q_OBJECT
public:
    VisibleOptionsMenu(QWidget *parent, KActionCollection *actions);

Q_SIGNALS:
    void visibleOptionsChanged();

private Q_SLOTS:
    void updateState();
    void toggleShowInfoBox(bool);
    void toggleShowCategory(bool);
    void toggleShowLabel(bool);
    void toggleShowDescription(bool);
    void toggleShowDate(bool);
    void toggleShowTime(bool);
    void toggleShowFilename(bool);
    void toggleShowEXIF(bool);
    void toggleShowImageSize(bool);
    void toggleShowRating(bool);

private:
    KToggleAction *m_showInfoBox;
    KToggleAction *m_showLabel;
    KToggleAction *m_showDescription;
    KToggleAction *m_showDate;
    KToggleAction *m_showTime;
    KToggleAction *m_showFileName;
    KToggleAction *m_showExif;
    KToggleAction *m_showImageSize;
    KToggleAction *m_showRating;
    QList<KToggleAction *> m_actionList;
};

}

#endif /* VISIBLEOPTIONSMENU_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
