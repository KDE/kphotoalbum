/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef VIEWER_VIDEODISPLAY_H
#define VIEWER_VIDEODISPLAY_H

#include "Display.h"
#include <kparts/componentfactory.h>
class QTimer;
class QHBoxLayout;

namespace Viewer
{

class VideoDisplay :public Viewer::Display
{
    Q_OBJECT

public:
    VideoDisplay( QWidget* parent );
    virtual bool setImage( DB::ImageInfoPtr info, bool forward );

signals:
    void stopped();

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFull();
    void zoomPixelForPixel();
    void play();
    void stop();
    void pause();
    void restart();

protected slots:
    void stateChanged( int );

protected:
    enum ErrorType { NoError, NoMimeType, NoKPart, NoLibrary, NoPartInstance,NoWidget };
    QString mimeTypeForFileName( const QString& fileName ) const;
    void showError( ErrorType, const QString& fileName, const QString& mimeType );
    void resize( float factor );
    virtual void resizeEvent( QResizeEvent* );
    void invokeKaffeineAction( const char* name );

private:
    KParts::ReadOnlyPart* _playerPart;
};

}

#endif /* VIEWER_VIDEODISPLAY_H */

