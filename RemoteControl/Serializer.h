/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONTROL_SERIALIZER_H
#define REMOTECONTROL_SERIALIZER_H

#include <QImage>
#include <QObject>

namespace RemoteControl
{

// We want to send the images from the database over as JPEG to get them over as fast as possible.
// On the other hand, JPEG do not support transparency, so the images used for example in the context menu,
// need to be send over as PNGs.
enum class ImageEncoding { PNG,
                           JPEG };

class SerializerInterface
{
public:
    virtual ~SerializerInterface() { }
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
    Serializer(QImage &value, ImageEncoding encoding)
        : m_image(value)
        , m_encoding(encoding)
    {
    }

    void encode(QDataStream &stream) override
    {
        m_image.save(stream.device(), m_encoding == ImageEncoding::JPEG ? "JPEG" : "PNG");
    }

    void decode(QDataStream &stream) override
    {
        m_image.load(stream.device(), m_encoding == ImageEncoding::JPEG ? "JPEG" : "PNG");
    }

private:
    QImage &m_image;
    ImageEncoding m_encoding;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SERIALIZER_H
