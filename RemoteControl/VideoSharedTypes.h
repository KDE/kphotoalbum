/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

constexpr int VIDEOPORT = 23457;
// 100K packages - Making them larger result in the stream of packages stopping mid way when served from a thread.
constexpr int PACKAGESIZE = 1024 * 100;

enum class PackageType { Header = 0,
                         Data = 1,
                         Cancel = 2 };
