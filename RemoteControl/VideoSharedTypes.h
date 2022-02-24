/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

constexpr int VIDEOPORT = 23457;
constexpr int PACKAGESIZE = 1024 * 1024; // 1Mbyte

enum class PackageType { Header = 0,
                         Data = 1 };
