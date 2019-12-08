if(WIN32 AND NOT CYGWIN)
    find_library(TAGLIB_LIBRARY tag)
    find_path(TAGLIB_INCLUDE_DIR taglib/tag.h)

    add_library(Taglib::Taglib STATIC IMPORTED GLOBAL)
    set_target_properties(Taglib::Taglib PROPERTIES
        IMPORTED_LOCATION ${TAGLIB_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES "${TAGLIB_INCLUDE_DIR}"
    )
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Taglib taglib REQUIRED IMPORTED_TARGET GLOBAL)
    add_library(Taglib::Taglib ALIAS PkgConfig::Taglib)
endif()
