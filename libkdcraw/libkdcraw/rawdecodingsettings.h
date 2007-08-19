/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2006-12-09
 * Description : Raw decoding settings
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

#ifndef RAW_DECODING_SETTINGS_H
#define RAW_DECODING_SETTINGS_H

// Local includes.

#include "libkdcraw_export.h"

namespace KDcrawIface
{

class LIBKDCRAW_EXPORT RawDecodingSettings
{

public:

    /** RAW decoding Interpolation methods */
    enum DecodingQuality 
    {
        BILINEAR = 0,
        VNG      = 2,
        AHD      = 3
    };

    /** Output RGB color space used to decoded image */ 
    enum OutputColorSpace 
    {
        RAWCOLOR = 0,
        SRGB,
        ADOBERGB,
        WIDEGAMMUT,
        PROPHOTO
    };

    /** Standard constructor with default settings */
    RawDecodingSettings()
    {
        sixteenBitsImage           = false;
        brightness                 = 1.0;
        RAWQuality                 = BILINEAR;
        outputColorSpace           = SRGB;
        RGBInterpolate4Colors      = false;
        DontStretchPixels          = false;
        unclipColors               = 0;
        cameraColorBalance         = true;
        automaticColorBalance      = true;
        halfSizeColorImage         = false;

        enableBlackPoint           = false;
        blackPoint                 = 0;

        enableNoiseReduction       = false;
        NRThreshold                = 100;

        enableColorMultipliers     = false;
        colorBalanceMultipliers[0] = 0.0;
        colorBalanceMultipliers[1] = 0.0;
        colorBalanceMultipliers[2] = 0.0;
        colorBalanceMultipliers[3] = 0.0;
    };
    
    /** Compare for equality */
    bool operator==(const RawDecodingSettings &o) const
    {
        return sixteenBitsImage == o.sixteenBitsImage
            && brightness == o.brightness
            && RAWQuality == o.RAWQuality
            && outputColorSpace == o.outputColorSpace
            && RGBInterpolate4Colors == o.RGBInterpolate4Colors  
            && DontStretchPixels == o.DontStretchPixels  
            && unclipColors == o.unclipColors  
            && cameraColorBalance == o.cameraColorBalance  
            && automaticColorBalance == o.automaticColorBalance  
            && halfSizeColorImage == o.halfSizeColorImage  
            && enableBlackPoint == o.enableBlackPoint  
            && blackPoint == o.blackPoint  
            && enableNoiseReduction == o.enableNoiseReduction  
            && NRThreshold == o.NRThreshold  
            && enableColorMultipliers == o.enableColorMultipliers  
            && colorBalanceMultipliers[0] == o.colorBalanceMultipliers[0]  
            && colorBalanceMultipliers[1] == o.colorBalanceMultipliers[1]  
            && colorBalanceMultipliers[2] == o.colorBalanceMultipliers[2]  
            && colorBalanceMultipliers[3] == o.colorBalanceMultipliers[3]  
          ;
    };

    /** Standard destructor */
    virtual ~RawDecodingSettings(){};

    /** Method to use a settings to optimize time loading, for exemple to compute image histogram */
    void optimizeTimeLoading(void)
    {
        sixteenBitsImage           = true;
        brightness                 = 1.0;
        RAWQuality                 = BILINEAR;
        outputColorSpace           = SRGB;
        RGBInterpolate4Colors      = false;
        DontStretchPixels          = false;
        unclipColors               = 0;
        cameraColorBalance         = true;
        automaticColorBalance      = true;
        halfSizeColorImage         = true;

        enableBlackPoint           = false;
        blackPoint                 = 0;

        enableNoiseReduction       = false;
        NRThreshold                = 100;

        enableColorMultipliers     = false;
        colorBalanceMultipliers[0] = 0.0;
        colorBalanceMultipliers[1] = 0.0;
        colorBalanceMultipliers[2] = 0.0;
        colorBalanceMultipliers[3] = 0.0;
    };

public:

    /** If true, decode RAW file in 16 bits per color per pixel else 8 bits.
    */
    bool sixteenBitsImage;    

    /** Half-size color image decoding (twice as fast as "enableRAWQuality"). 
        Use this option to reduce time loading to render histogram for example, 
        no to render an image to screen. 
    */
    bool halfSizeColorImage;

    /**  Use the color balance specified by the camera. If this can't be found, 
         reverts to the default. 
    */
    bool cameraColorBalance;
    
    /** Automatic color balance. The default is to use a fixed color balance 
        based on a white card photographed in sunlight. 
    */
    bool automaticColorBalance;
    
    /** RAW file decoding using RGB interpolation as four colors. 
    */
    bool RGBInterpolate4Colors;

    /** For cameras with non-square pixels, do not stretch the image to its 
        correct aspect ratio. In any case, this option guarantees that each 
        output pixel corresponds to one RAW pixel. 
    */
    bool DontStretchPixels;
    
    /** Unclip Highlight color level:
        0   = Clip all highlights to solid white.
        1   = Leave highlights unclipped in various shades of pink.
        2   = Blend clipped and unclipped values together for a gradual
              fade to white.
        3-9 = Reconstruct highlights. Low numbers favor whites; high numbers
              favor colors.
    */
    int unclipColors;

    /** RAW quality decoding factor value. See DecodingQuality values 
        for details. 
    */
    DecodingQuality RAWQuality;

    /** Use wavelets to erase noise while preserving real detail. 
    */
    bool enableNoiseReduction;

    /** Noise reduction threshold value.  
        The best threshold should be somewhere between 100 and 1000.
    */
    int NRThreshold;
    
    /** Brightness of output image. 
    */
    float brightness;   

    /** Set on the black point setting to decode RAW image.
    */
    bool enableBlackPoint;

    /** Black Point value of output image. 
    */
    int blackPoint;   

    /** The output color space used to decoded RAW data. See OutputColorSpace 
        values for details. 
    */
    OutputColorSpace outputColorSpace;

    /** Set on the Raw color balance multipliers settings to decode RAW image.
    */
    bool enableColorMultipliers;

    /** Raw color balance multipliers used with option '-r'. Theses values 
        are applied to the input RAW image in the camera's raw color space.
        By order multipliers are:  

        - colorBalanceMultipliers[0] = red multiplier.
        - colorBalanceMultipliers[1] = green1 multiplier.
        - colorBalanceMultipliers[2] = blue multiplier.
        - colorBalanceMultipliers[3] = green2 multiplier.

        If green2 is zero, it is assumed to be the same as green1.  Multiplying 
        all values by a constant has no effect.  Thus all these are the same:

        1.6, 1, 1.2, 1
        3.2, 2, 2.4, 2
        3.2, 2, 2.4, 0

        The Bayer pattern in an RGB camera is always:

        [ red  ][green1]
        [green2][ blue ]
    */
    double colorBalanceMultipliers[4];
};

}  // namespace KDcrawIface

#endif /* RAW_DECODING_SETTINGS_H */
