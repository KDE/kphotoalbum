/* SPDX-FileCopyrightText: 2003-2018 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ShowSelectionOnlyManager.h"

AnnotationDialog::ShowSelectionOnlyManager &AnnotationDialog::ShowSelectionOnlyManager::instance()
{
    static ShowSelectionOnlyManager instance;
    return instance;
}

AnnotationDialog::ShowSelectionOnlyManager::ShowSelectionOnlyManager()
    : m_limit(false)
{
}

bool AnnotationDialog::ShowSelectionOnlyManager::selectionIsLimited() const
{
    return m_limit;
}

void AnnotationDialog::ShowSelectionOnlyManager::toggle()
{
    m_limit = !m_limit;
    if (m_limit)
        emit limitToSelected();
    else
        emit broaden();
}

void AnnotationDialog::ShowSelectionOnlyManager::unlimitFromSelection()
{
    if (m_limit) {
        m_limit = false;
        emit broaden();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ShowSelectionOnlyManager.cpp"
