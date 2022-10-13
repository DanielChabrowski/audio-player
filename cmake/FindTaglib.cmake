if(WIN32 AND NOT CYGWIN)
    find_library(TAGLIB_LIBRARY tag)
    find_path(TAGLIB_INCLUDE_DIR taglib/tag.h)

    add_library(Taglib::Taglib STATIC IMPORTED GLOBAL)
    set_target_properties(
        Taglib::Taglib
        PROPERTIES
            IMPORTED_LOCATION ${TAGLIB_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES "${TAGLIB_INCLUDE_DIR}"
    )
else()
    # find_package(PkgConfig REQUIRED)
    # pkg_check_modules(Taglib taglib REQUIRED IMPORTED_TARGET GLOBAL)

    add_library(TagLib SHARED IMPORTED)
    set_target_properties(
        TagLib
        PROPERTIES
            IMPORTED_LOCATION
                /home/dante/projects/taglib/build/taglib/Release/libtag.so
    )

    target_link_libraries(TagLib INTERFACE libz.so)

    target_include_directories(
        TagLib
        INTERFACE
            /home/dante/projects/taglib
            /home/dante/projects/taglib/taglib
            /home/dante/projects/taglib/taglib/toolkit
    )
    add_library(Taglib::Taglib ALIAS TagLib)
endif()
