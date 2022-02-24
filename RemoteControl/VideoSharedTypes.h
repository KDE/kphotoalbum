#pragma once

constexpr int VIDEOPORT = 23457;
constexpr int PACKAGESIZE = 1024 * 1024; // 1Mbyte

enum class PackageType { Header = 0,
                         Data = 1 };
