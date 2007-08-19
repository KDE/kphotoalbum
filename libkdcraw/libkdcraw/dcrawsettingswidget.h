/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2006-09-13
 * Description : dcraw settings widgets
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

#ifndef DCRAW_SETTINGS_WIDGET_H
#define DCRAW_SETTINGS_WIDGET_H

// KDE includes.

#include <ktabwidget.h>

// Local Includes.

#include "libkdcraw_export.h"
#include "rawdecodingsettings.h"

namespace KDcrawIface
{

class DcrawSettingsWidgetPriv;

class LIBKDCRAW_EXPORT DcrawSettingsWidget : public KTabWidget
{
    Q_OBJECT
    
public:

    DcrawSettingsWidget(QWidget *parent, 
                        bool sixteenBitsOption=false, 
                        bool outputColorSpaceOption=true, 
                        bool showAdvancedOptions=false);
    virtual ~DcrawSettingsWidget();

    bool   sixteenBits();
    bool   useCameraWB();
    bool   useAutoColorBalance();
    bool   useFourColor();
    bool   useDontStretchPixels();
    bool   useNoiseReduction();
    bool   useBlackPoint();
    int    unclipColor();
    double brightness();
    int    blackPoint();
    int    NRThreshold();
    bool   useColorMultipliers();
    double colorMultiplier1();
    double colorMultiplier2();
    double colorMultiplier3();
    double colorMultiplier4();

    RawDecodingSettings::DecodingQuality  quality();
    RawDecodingSettings::OutputColorSpace outputColorSpace();

    void   setSixteenBits(bool b);
    void   setCameraWB(bool b);
    void   setAutoColorBalance(bool b);
    void   setFourColor(bool b);
    void   setDontStretchPixels(bool b);
    void   setNoiseReduction(bool b);
    void   setUseBlackPoint(bool b);
    void   setUnclipColor(int v);
    void   setBrightness(double b);
    void   setBlackPoint(int b);
    void   setNRThreshold(int b);
    void   setQuality(RawDecodingSettings::DecodingQuality q);
    void   setOutputColorSpace(RawDecodingSettings::OutputColorSpace c);
    void   setUseColorMultipliers(bool b);
    void   setcolorMultiplier1(double b);
    void   setcolorMultiplier2(double b);
    void   setcolorMultiplier3(double b);
    void   setcolorMultiplier4(double b);

    void   setDefaultSettings();

Q_SIGNALS:

    void signalSixteenBitsImageToggled(bool);

private Q_SLOTS:

    void slotsixteenBitsImageToggled(bool);
    void slotColorMultToggled(bool);
    void slotUnclipColorActivated(int);
    void slotNoiseReductionToggled(bool);
    void processDcrawUrl(const QString&);

private:

    DcrawSettingsWidgetPriv* d;
};

} // NameSpace KDcrawIface

#endif /* DCRAW_SETTINGS_WIDGET_H */
