// SPDX-FileCopyrightText: 2009, 2013 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013, 2015-2016, 2018-2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: CC-BY-SA-4.0

/**
\page coding-standards Coding Standards

<h2>Compatibility</h2>

Not every distro ships the latest version of Qt. As a rule of thumb, we aim for compatibility with the latest Ubuntu LTS release.
Since invent.kde.org makes it easier to get a continuous integration pipeline for Debian stable, we are currently supporting that as well.

<h3>Why Ubuntu?</h3>

\li It is one of the more popular distros
\li The release life cycle is very regular
\li Package information is readily available. See e.g. the information for
    <a href='https://packages.ubuntu.com/libqt5gui' alt='Ubuntu package search'>libqt5gui</a> or
    <a href='https://packages.ubuntu.com/libkf5kio-dev' alt='Ubuntu package search'>libkf5kio-dev</a>

<h2>Code formatting</h2>

We use clang-format to change our code. Please do use our commit-hooks that are described in `dev/README-dev.txt`.

<h2>General conventions</h2>

Here are a few pointers regarding coding standards in KPhotoAlbum. Please do stick to them, so the
code is easier to get into for new people, and easier to maintain for everyone.

For best results and minimum hassle, please do use clang-format to format your code:
KPhotoAlbum ships with a `.clang-format` file.

\li Basically, we try to stick by the
    <a href="https://community.kde.org/Policies/Frameworks_Coding_Style">kdelibs coding style</a>.
\li Instance variables are prefixed with `m_` (not only with `_`).
\li Static instance variables are prefixed with `s_`.
\li Methods that are overridden from a superclass should be marked as such using the C++11
    `override` keyword.
\li KPhotoAlbum is warning free zone. Please keep it that way. No warnings during compilations are
    accepted.
\li Don't enable debugging statements in production code.
    Use categorized logging (QLoggingCategory) with the prefix "kphotoalbum.DIRECTORY" and possibly subcomponents.
    You can then enable the code by setting the environment variable QT_LOGGING_RULES="kphotoalbum.*=true".

<h2>Include files and forward declarations</h2>

The order of include statements should always be the following:

\li "config-kpa-xxx.h" includes
\li file-specific header file
\li KPhotoAlbum-includes
\li other includes

To speed up compilation and make things easier to understand, you should be careful about what you
include, and when cleaning up code, please check whether you need all the include files.

In header files you should try hard to see if you really need an include file at all, or whether
you can get by with only a forward declaration.

A forward declaration looks like:

\code
class MyClass;
namespace MyNameSpace { class MyClass; }
\endcode

You can get by with only a forward declaration when all you do is one of these:

\li declare a method that passes a pointer or reference as argument (`doSomething(const MyClass&)`)
\li return an object from a method (`MyClass getClass()`)
\li you only have the class in a container (`QList<MyClass>`)

In contrast you do need the include files when:

\li you declare a method that takes a value argument (`doSomething(MyClass cls)`)
\li you have an instance variable of the given class (`MyClass m_class`).

*/
// vi:expandtab:tabstop=4 shiftwidth=4:
