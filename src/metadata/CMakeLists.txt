find_package(Taglib REQUIRED)
find_package(Qt6 COMPONENTS Sql CONFIG REQUIRED)

set(SOURCES
    AudioMetaData.hpp
    IAudioMetaDataProvider.hpp
    AudioMetaDataProvider.cpp
    AudioMetaDataProvider.hpp
    MetaDataCache.cpp
    MetaDataCache.hpp
)

add_library(metadata ${SOURCES})
add_library(player::metadata ALIAS metadata)

target_link_libraries(metadata PUBLIC Qt::Sql Qt::Core PRIVATE Taglib::Taglib)

target_include_directories(metadata PUBLIC ${CMAKE_CURRENT_LIST_DIR})
