/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef STATUSBAR_H
#define STATUSBAR_H
#include "BreadcrumbViewer.h"
#include <KStatusBar>
class QToolButton;
class QTimer;
class QProgressBar;
class QLabel;

namespace MainWindow {
class ImageCounter;
class DirtyIndicator;

class StatusBar :public KStatusBar
{
    Q_OBJECT
public:
    StatusBar();
    DirtyIndicator* _dirtyIndicator;
    ImageCounter* _partial;
    BreadcrumbViewer* _pathIndicator;

    void setLocked( bool locked );
    void startProgress( const QString& text, int total );
    void setProgress( int progress );
    void setProgressBarVisible( bool );

signals:
    void cancelRequest();

private slots:
    void hideStatusBar();
    void showStatusBar();

private:
    void setupGUI();
    void setPendingShow();

    QLabel* _lockedIndicator;
    QProgressBar* m_progressBar;
    QToolButton* m_cancel;
    QTimer* m_pendingShowTimer;
};

}


#endif /* STATUSBAR_H */

