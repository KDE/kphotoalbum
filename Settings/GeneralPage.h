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
#ifndef GENERALPAGE_H
#define GENERALPAGE_H
#include <QWidget>

class QComboBox;
class QSpinBox;
class QCheckBox;
class KComboBox;
namespace Settings
{
class SettingsData;

class GeneralPage :public QWidget
{
    Q_OBJECT
public:
    GeneralPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );
    void setUseRawThumbnailSize( const QSize& size );
    QSize useRawThumbnailSize();

private slots:
    void showHistogramChanged( int state ) const;

private:
    KComboBox* _trustTimeStamps;
    QCheckBox* _useEXIFRotate;
    QCheckBox* _useEXIFComments;
    QCheckBox* _useRawThumbnail;
    QSpinBox* _useRawThumbnailWidth;
    QSpinBox* _useRawThumbnailHeight;
    QCheckBox* _showHistogram;
    QSpinBox* _barWidth;
    QSpinBox* _barHeight;
    QCheckBox* _showSplashScreen;
    QComboBox* _albumCategory;
};
}


#endif /* GENERALPAGE_H */

