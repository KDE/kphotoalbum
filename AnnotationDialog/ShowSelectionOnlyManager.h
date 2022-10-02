// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ANNOTATIONDIALOG_SHOWSELECTIONONLYMANAGER_H
#define ANNOTATIONDIALOG_SHOWSELECTIONONLYMANAGER_H

#include <qobject.h>

namespace AnnotationDialog
{
class ShowSelectionOnlyManager : public QObject
{
    Q_OBJECT

public:
    static ShowSelectionOnlyManager &instance();
    bool selectionIsLimited() const;

public Q_SLOTS:
    void toggle();
    void unlimitFromSelection();

Q_SIGNALS:
    void limitToSelected();
    void broaden();

private:
    ShowSelectionOnlyManager();
    bool m_limit;
};

}

#endif /* ANNOTATIONDIALOG_SHOWSELECTIONONLYMANAGER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
