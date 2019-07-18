/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef REMOTECONTROL_SERIALIZER_H
#define REMOTECONTROL_SERIALIZER_H

#include <QObject>

namespace RemoteControl
{

enum class BackgroundType { Transparent,
                            NonTransparent };

static void fastStreamImage(QDataStream &stream, const QImage &image, BackgroundType type)
{
    if (type == BackgroundType::Transparent) {
        QImage result(image.width(), image.height(), QImage::Format_RGB32);
        result.fill(Qt::black);
        QPainter p(&result);
        p.drawImage(0, 0, image);
        p.end();
        result.save(stream.device(), "JPEG");
    } else
        image.save(stream.device(), "JPEG");
}

class SerializerInterface
{
public:
    virtual ~SerializerInterface() {}
    virtual void encode(QDataStream &) = 0;
    virtual void decode(QDataStream &) = 0;
};

template <class T>
class Serializer : public SerializerInterface
{
public:
    Serializer(T &value)
        : m_value(value)
    {
    }
    void encode(QDataStream &stream) override
    {
        stream << m_value;
    }
    void decode(QDataStream &stream) override
    {
        stream >> m_value;
    }

private:
    T &m_value;
};

template <>
class Serializer<QImage> : public SerializerInterface
{
public:
    Serializer(QImage &value, BackgroundType background = BackgroundType::NonTransparent)
        : m_image(value)
        , m_background(background)
    {
    }
    void encode(QDataStream &stream) override
    {
        fastStreamImage(stream, m_image, m_background);
    }
    void decode(QDataStream &stream) override
    {
        m_image.load(stream.device(), "JPEG");
    }

private:
    QImage &m_image;
    BackgroundType m_background;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SERIALIZER_H
