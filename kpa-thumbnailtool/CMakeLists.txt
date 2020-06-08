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

# vi:expandtab:tabstop=4 shiftwidth=4:
