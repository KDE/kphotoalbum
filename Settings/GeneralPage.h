/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef GENERALPAGE_H
#define GENERALPAGE_H
#include <QWidget>

class QComboBox;
class QSpinBox;
class QCheckBox;
class KComboBox;
class QTextEdit;
namespace Settings
{
class SettingsData;

class GeneralPage : public QWidget
{
    Q_OBJECT
public:
    explicit GeneralPage(QWidget *parent);
    void loadSettings(Settings::SettingsData *);
    void saveSettings(Settings::SettingsData *);
    void setUseRawThumbnailSize(const QSize &size);
    QSize useRawThumbnailSize();

private slots:
    void showHistogramChanged(int state) const;
    void useEXIFCommentsChanged(int state);
    void stripEXIFCommentsChanged(int state);

private:
    KComboBox *m_trustTimeStamps;
    QCheckBox *m_useEXIFRotate;
    QCheckBox *m_useEXIFComments;
    QTextEdit *m_commentsToStrip;
    QCheckBox *m_stripEXIFComments;
    QCheckBox *m_useRawThumbnail;
    QSpinBox *m_useRawThumbnailWidth;
    QSpinBox *m_useRawThumbnailHeight;
    QCheckBox *m_showHistogram;
    QCheckBox *m_histogramUseLinearScale;
    QSpinBox *m_barWidth;
    QSpinBox *m_barHeight;
    QCheckBox *m_showSplashScreen;
#ifdef KPA_ENABLE_REMOTECONTROL
    QCheckBox *m_listenForAndroidDevicesOnStartup;
#endif
};
}

#endif /* GENERALPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
