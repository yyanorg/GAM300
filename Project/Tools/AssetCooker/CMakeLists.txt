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

# The runtime libraries have to sit beside this exe, and cannot be inherited
# from the Editor target.
#
# Editor/CMakeLists.txt copies fmod.dll and the vcpkg DLLs into the output
# directory as POST_BUILD steps on Editor, and the release workflow builds
# --target AssetCooker without ever building Editor, so none of them ran. On
# Windows a missing DLL kills the process before main() with no message at all:
# the cook step failed in 0.7 seconds having printed nothing, which reads as
# the cooker rejecting its arguments rather than never having started.
if(WIN32)
    add_custom_command(TARGET AssetCooker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Libraries/FMOD/core/lib/x64/fmod.dll
        "${OUTPUT_DIR}"
    )
    add_custom_command(TARGET AssetCooker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:EditorDebug>>,${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin,${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin>"
        "${OUTPUT_DIR}"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_custom_command(TARGET AssetCooker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Libraries/FMOD/core/lib/x64/libfmod.so.14.9
        "${OUTPUT_DIR}"
    )
endif()
