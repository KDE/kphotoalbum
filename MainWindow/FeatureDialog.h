/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef FEATUREDIALOG_H
#define FEATUREDIALOG_H
#include <kdialogbase.h>
#include <qtextbrowser.h>

namespace MainWindow
{

class FeatureDialog : public KDialogBase {
    Q_OBJECT

public:
    FeatureDialog( QWidget* parent, const char* name = 0 );
    static bool hasAllFeaturesAvailable();
    static QString featureString();
    static bool hasVideoSupport( const QString& mimeType );

protected:
    static bool hasKIPISupport();
    static bool hasSQLDBSupport();
    static bool hasEXIV2Support();
    static bool hasEXIV2DBSupport();
    static bool hasRAWSupport();
};

class HelpBrowser :public QTextBrowser
{
public:
    HelpBrowser( QWidget* parent, const char* name = 0 );
    virtual void setSource( const QString& name );
};

}

#endif /* FEATUREDIALOG_H */

