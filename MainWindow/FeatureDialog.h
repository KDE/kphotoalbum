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
#ifndef FEATUREDIALOG_H
#define FEATUREDIALOG_H
#include <KTextBrowser>
#include <kdialog.h>

namespace MainWindow
{

class FeatureDialog : public KDialog {
    Q_OBJECT

public:
    FeatureDialog( QWidget* parent );
    static bool hasAllFeaturesAvailable();
    static QString featureString();
    static QStringList supportedVideoMimeTypes();

protected:
    static bool hasKIPISupport();
    static bool hasSQLDBSupport();
    static bool hasEXIV2Support();
    static bool hasEXIV2DBSupport();
};

class HelpBrowser :public KTextBrowser
{
public:
    explicit HelpBrowser( QWidget* parent, const char* name = 0 );
    virtual void setSource( const QUrl& name );
};

}

#endif /* FEATUREDIALOG_H */

