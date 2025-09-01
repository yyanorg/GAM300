# Force CMake to reconfigure when source directory changes
file(GLOB_RECURSE SOURCE_DEPS "src/*" "include/*")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${SOURCE_DEPS})

# Automatically find all source files recursively
file(GLOB_RECURSE EDITOR_SOURCES
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE EDITOR_HEADERS
    "include/*.h"
    "include/*.hpp"
    "src/*.h"
    "src/*.hpp"
)

message(STATUS "Building Editor executable for ${CMAKE_BUILD_TYPE}")

# Editor is always an executable
add_executable(Editor ${EDITOR_SOURCES} ${EDITOR_HEADERS})

# Automatically include all subdirectories
file(GLOB_RECURSE EDITOR_INCLUDE_SUBDIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*"
)
set(EDITOR_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
foreach(item ${EDITOR_INCLUDE_SUBDIRS})
    if(IS_DIRECTORY ${item})
        list(APPEND EDITOR_INCLUDE_DIRS ${item})
    endif()
endforeach()

target_include_directories(Editor
    PRIVATE
        ${EDITOR_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Engine/include  
        ${CMAKE_SOURCE_DIR}/Game/include
        ${CMAKE_SOURCE_DIR}/Libraries/ImGui
        ${CMAKE_SOURCE_DIR}/Libraries/ImGui/backends
        ${CMAKE_SOURCE_DIR}/Libraries/glad/include
        ${CMAKE_SOURCE_DIR}/Libraries/glfw/include
        ${CMAKE_SOURCE_DIR}/Libraries/glm
)

# Link to Engine (shared lib) and Game (static lib)
target_link_libraries(Editor
    PRIVATE
        Engine  # shared library
        Game    # static library
)