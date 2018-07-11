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
#ifndef IMAGEREQUEST_H
#define IMAGEREQUEST_H
#include <qstring.h>
#include <qsize.h>
#include <QHash>
#include "enums.h"
#include <DB/FileName.h>

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This class is shared among the image loader thead and the GUI tread, if
// you don't know the implication of this stay out of this class!
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

namespace ImageManager
{
class ImageClientInterface;

class ImageRequest {
public:
    ImageRequest( const DB::FileName& fileName, const QSize& size, int angle, ImageClientInterface* client);
    virtual ~ImageRequest() {}
    ImageRequest( bool requestExit );

    bool isNull() const;

    /** This is the filename that the media is known by in the database.
        See \ref fileSystemFileName for details
    **/
    DB::FileName databaseFileName() const;

    /**
        This is the file name that needs to be loaded using the image loader.
        In case of a video file where we are loading the snapshot from a prerendered
        image, this file name may be different than the one returned from dabataseFileName.
        In that example, databaseFileName() returns the path to the video file,
        while fileSystemFileName returns the path to the prerendered image.
    **/
    virtual DB::FileName fileSystemFileName() const;

    int width() const;
    int height() const;
    QSize size() const;
    int angle() const;

    ImageClientInterface* client() const;

    QSize fullSize() const;
    void setFullSize( const QSize& );
    void setLoadedOK( bool ok );
    bool loadedOK() const;

    void setPriority( const Priority prio );
    Priority priority() const;

    bool operator<( const ImageRequest& other ) const;
    bool operator==( const ImageRequest& other ) const;

    virtual bool stillNeeded() const;

    bool doUpScale() const;
    void setUpScale( bool b );

    void setIsThumbnailRequest( bool );
    bool isThumbnailRequest() const;
    bool isExitRequest() const;

private:
    bool m_null;
    DB::FileName m_fileName;

    int m_width;
    int m_height;
    ImageClientInterface* m_client;
    int m_angle;
    QSize m_fullSize;
    Priority m_priority;
    bool m_loadedOK;
    bool m_dontUpScale;
    bool m_isThumbnailRequest;
    bool m_isExitRequest;
};

inline uint qHash(const ImageRequest& ir)
{
    return DB::qHash(ir.databaseFileName()) ^ ::qHash(ir.width()) ^ ::qHash(ir.angle());
}

}

#endif /* IMAGEREQUEST_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
