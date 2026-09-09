# AssetCooker: the asset compilation the editor does, runnable from a command
# line so CI can cook as a build step instead of relying on a developer's disk.
#
# Built only for Editor configurations, and this is not arbitrary: the asset
# discovery and compilation in MetaFilesManager::InitializeAssetMetaFiles is
# inside #ifdef EDITOR, so an Engine built for a Game config walks the asset
# tree and does nothing. The cooker therefore links the Editor-flavoured
# Engine. It still produces desktop assets; --android produces the Android set.
if(ANDROID)
    return()
endif()
if(NOT CMAKE_BUILD_TYPE MATCHES "Editor")
    return()
endif()

add_executable(AssetCooker main.cpp)

target_link_libraries(AssetCooker PRIVATE Engine)

set_target_properties(AssetCooker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)
