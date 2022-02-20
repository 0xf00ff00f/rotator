cmake_minimum_required(VERSION 3.10)

project(game)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rdparty)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    find_package(OpenGL REQUIRED)
    find_package(SDL REQUIRED)
    find_package(GLEW REQUIRED)
endif()

set(SOURCES
    main.cc
    demo.cc
    mesh.cc
    wobble.cc
    texture.cc
    lazytexture.cc
    textureatlaspage.cc
    textureatlas.cc
    fontcache.cc
    ioutil.cc
    shaderprogram.cc
    shadermanager.cc
    spritebatcher.cc
    uipainter.cc
)

add_executable(game ${SOURCES})

target_link_libraries(game
PUBLIC
    glm
    stb
)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_libraries(game
    PUBLIC
        GLEW::GLEW
        OpenGL::GL
        SDL::SDL
    )
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(game PROPERTIES LINK_FLAGS "-s FULL_ES3=1 --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets")
endif()

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_custom_command(TARGET game
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
    )
endif()
