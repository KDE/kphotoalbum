/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWERSIZECONFIG_H
#define VIEWERSIZECONFIG_H

#include <QGroupBox>
class QCheckBox;
class QSpinBox;

namespace Settings
{

class ViewerSizeConfig : public QGroupBox
{
    Q_OBJECT

public:
    ViewerSizeConfig(const QString &title, QWidget *parent);
    void setSize(const QSize &size);
    QSize size();
    void setLaunchFullScreen(bool b);
    bool launchFullScreen() const;

private:
    QCheckBox *m_fullScreen;
    QSpinBox *m_width;
    QSpinBox *m_height;
};
}

#endif /* VIEWERSIZECONFIG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
