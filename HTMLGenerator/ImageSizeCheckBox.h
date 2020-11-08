/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef HTMLGENERATOR_IMAGESIZECHECKBOX_H
#define HTMLGENERATOR_IMAGESIZECHECKBOX_H

#include <qcheckbox.h>

namespace HTMLGenerator
{
class ImageSizeCheckBox : public QCheckBox
{

public:
    ImageSizeCheckBox(int width, int height, QWidget *parent)
        : QCheckBox(QString::fromLatin1("%1x%2").arg(width).arg(height), parent)
        , m_width(width)
        , m_height(height)
    {
    }

    ImageSizeCheckBox(const QString &text, QWidget *parent)
        : QCheckBox(text, parent)
        , m_width(-1)
        , m_height(-1)
    {
    }

    int width() const
    {
        return m_width;
    }
    int height() const
    {
        return m_height;
    }
    QString text(bool withOutSpaces) const
    {
        return text(m_width, m_height, withOutSpaces);
    }
    static QString text(int width, int height, bool withOutSpaces)
    {
        if (width == -1)
            if (withOutSpaces)
                return QString::fromLatin1("fullsize");
            else
                return QString::fromLatin1("full size");

        else
            return QString::fromLatin1("%1x%2").arg(width).arg(height);
    }

    bool operator<(const ImageSizeCheckBox &other) const
    {
        return m_width < other.width();
    }

private:
    int m_width;
    int m_height;
};
}

#endif /* HTMLGENERATOR_IMAGESIZECHECKBOX_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
