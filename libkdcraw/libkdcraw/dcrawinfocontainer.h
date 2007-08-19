/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-05-02
 * Description : RAW file identification information container 
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DCRAW_INFO_CONTAINER_H
#define DCRAW_INFO_CONTAINER_H

// QT includes.

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QSize>

// Local Includes.

#include "libkdcraw_export.h"

namespace KDcrawIface
{

class LIBKDCRAW_EXPORT DcrawInfoContainer
{

public:

    /** Standard constructor */
    DcrawInfoContainer()
    {
        sensitivity       = -1;
        exposureTime      = -1.0;
        aperture          = -1.0;
        focalLength       = -1.0;
        pixelAspectRatio  = 1.0;    // Default value. This can be unavailable (depending of camera model).
        rawColors         = -1;
        rawImages         = -1;
        hasIccProfile     = false;
        isDecodable       = false;
        hasSecondaryPixel = false;
        daylightMult[0]   = 0.0;
        daylightMult[1]   = 0.0;
        daylightMult[2]   = 0.0;
        cameraMult[0]     = 0.0;
        cameraMult[1]     = 0.0;
        cameraMult[2]     = 0.0;
        cameraMult[3]     = 0.0;
    };

    /** Standard destructor */
    virtual ~DcrawInfoContainer(){}
    
    /** return 'true' if container is empty, else 'false' */    
    bool isEmpty()
    {
        if ( make.isEmpty()          &&
             model.isEmpty()         &&
             filterPattern.isEmpty() &&
             DNGVersion.isEmpty()    &&
             exposureTime == -1.0    && 
             aperture     == -1.0    && 
             focalLength  == -1.0    && 
             pixelAspectRatio == 1.0 &&
             sensitivity  == -1      && 
             rawColors    == -1      &&
             rawImages    == -1      &&
             !dateTime.isValid()     && 
             !imageSize.isValid() )
            return true;
        else
            return false;
    };
    
    /** True if camera sensor use a secondary pixel. */
    bool      hasSecondaryPixel;  
    /** True if RAW file include an ICC color profile. */
    bool      hasIccProfile;      
    /** True is RAW file is decodable by dcraw. */
    bool      isDecodable;        

    /** The number of RAW colors. */
    int       rawColors;          
    
    /** The number of RAW images. */
    int       rawImages;

    /** The sensitivity in ISO used by camera to take the picture. */
    long      sensitivity;        

    /** ==> 1/exposureTime = exposure time in seconds. */
    float     exposureTime;       
    /** ==> Aperture value in APEX. */
    float     aperture;           
    /** ==> Focal Length value in mm. */
    float     focalLength;        
    /** The pixel Aspect Ratio if != 1.0. NOTE: if == 1.0, dcraw do not show this value. */
    float     pixelAspectRatio;   

    /** White color balance settings. */
    double    daylightMult[3];    
    /** Camera multipliers used for White Balance adjustements */
    double    cameraMult[4];

    /** The camera maker. */
    QString   make;               
    /** The camera model. */
    QString   model;              
    /** The artist name who have picture owner. */
    QString   owner;       
    /** The demosaising filter pattern. */
    QString   filterPattern;      
    /** The DNG version. NOTE: only show with DNG RAW files. */
    QString   DNGVersion;         

    /** Date & time when have been taken the picture. */
    QDateTime dateTime;           

    /** The image dimensions in pixels. */
    QSize     imageSize;          
};

} // namespace KDcrawIface

#endif /* DCRAW_INFO_CONTAINER_H */
