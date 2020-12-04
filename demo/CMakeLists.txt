# SPDX-FileCopyrightText: 2007 Jesper K. Pedersen <blackie@kde.org>
# SPDX-FileCopyrightText: 2016, 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory( CategoryImages )

########### install files ###############
#exif-info.db
install(
    FILES
    index.xml
    anne_helene.jpg
    bar55_2.jpg
    bar55.jpg
    blackie.jpg
    cold_water.jpg
    donna_2.jpg
    donna.jpg
    grand_canyon_1.jpg
    grand_canyon_2.jpg
    grand_canyon_3.jpg
    jesper_2.jpg
    jesper.jpg
    new_wave_1.jpg
    new_wave_2.jpg
    pool_2.jpg
    pool.jpg
    qt-logo.jpg
    skagen_2.jpg
    skagen.jpg
    snow.jpg
    spiff_2.jpg
    spiff.jpg
    sunset.jpg
    wayne.jpg
    movie.avi
    DESTINATION "${KDE_INSTALL_DATADIR}/kphotoalbum/demo"
    )

# vi:expandtab:tabstop=4 shiftwidth=4:
