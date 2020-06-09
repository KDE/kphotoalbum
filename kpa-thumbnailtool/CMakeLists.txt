add_executable(kpa-thumbnailtool
    main.cpp
    Logging.cpp
    Logging.h
    ThumbnailCacheConverter.cpp
    ThumbnailCacheConverter.h
    )
target_link_libraries(kpa-thumbnailtool PRIVATE
    KPA::Thumbnails
    KF5::CoreAddons
    )

install(TARGETS kpa-thumbnailtool ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# vi:expandtab:tabstop=4 shiftwidth=4:
