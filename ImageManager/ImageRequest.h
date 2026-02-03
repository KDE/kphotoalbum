// SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEREQUEST_H
#define IMAGEREQUEST_H
#include "enums.h"

#include <kpabase/FileName.h>

#include <QHash>
#include <qsize.h>
#include <qstring.h>

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This class is shared among the image loader thead and the GUI tread, if
// you don't know the implication of this stay out of this class!
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING

namespace ImageManager
{
class ImageClientInterface;

class ImageRequest
{
public:
    enum class RequestType {
        ImageRequest,
        ExitRequest
    };
    ImageRequest(const DB::FileName &fileName, const QSize &size, int angle, ImageClientInterface *client);
    virtual ~ImageRequest() { }
    /**
     * @brief Create a special request.
     * This constructor can be used to create an ExitRequest.
     * You must not use this constructor with a request Type of ImageRequest - use the file name based constructor for an actual ImageRequest instead.
     * @param type ExitRequest
     */
    ImageRequest(RequestType type);

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

    ImageClientInterface *client() const;

    QSize fullSize() const;
    void setFullSize(const QSize &);
    void setLoadedOK(bool ok);
    bool loadedOK() const;

    void setPriority(const Priority prio);
    Priority priority() const;

    bool operator<(const ImageRequest &other) const;
    bool operator==(const ImageRequest &other) const;

    virtual bool stillNeeded() const;

    bool doUpScale() const;
    void setUpScale(bool b);

    void setIsThumbnailRequest(bool);
    bool isThumbnailRequest() const;
    bool isExitRequest() const;

    /**
     * @brief imageIsPreRotated is set when the loaded image is already rotated correctly.
     * This is usually the case for decoded raw images.
     * @return \c true, if the image is already rotated and does not need additional rotation, \c false otherwise
     */
    bool imageIsPreRotated() const;
    void setImageIsPreRotated(bool imageIsPreRotated);

private:
    const RequestType m_type;
    DB::FileName m_fileName;

    int m_width;
    int m_height;
    ImageClientInterface *m_client;
    int m_angle;
    QSize m_fullSize;
    Priority m_priority;
    bool m_loadedOK;
    bool m_dontUpScale;
    bool m_isThumbnailRequest;
    bool m_imageIsPreRotated;
};

inline uint qHash(const ImageRequest &ir)
{
    return DB::qHash(ir.databaseFileName()) ^ ::qHash(ir.width()) ^ ::qHash(ir.angle());
}

}

#endif /* IMAGEREQUEST_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
