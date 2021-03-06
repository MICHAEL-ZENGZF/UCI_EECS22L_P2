cmake_minimum_required(VERSION 2.8)

project(kchat-client)

SET(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_C_COMPILER "/usr/bin/gcc")
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")

add_definitions(-DSUPEROLD)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
    set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -pthread")
else()
    set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -pthread")
endif()

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory("${PROJECT_SOURCE_DIR}/src/AI")

find_package (PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK2 REQUIRED 
gtk+-2.0 
gdk-pixbuf-2.0)
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

FIND_PACKAGE(GTK2 REQUIRED)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include" 
${GTK2_INCLUDE_DIRS}
${GLIB_INCLUDE_DIRS})

LINK_DIRECTORIES("${PROJECT_SOURCE_DIR}/lib" 
"${PROJECT_SOURCE_DIR}/src/AI"
${GTK2_LINK_FLAGS}
)

LINK_LIBRARIES(
${GTK2_LIBRARIES}
${GTK2_LINK_FLAGS}
${LINK_DIRECTORIES} 
glib-2.0
gthread-2.0
${GLIB_LDFLAGS}
AI_LIB)

AUX_SOURCE_DIRECTORY("${PROJECT_SOURCE_DIR}/src" DIR_SRCS )
add_executable(${PROJECT_NAME} ${DIR_SRCS})

add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res"
        "${EXECUTABLE_OUTPUT_PATH}/res")

# set(SOURCE src/SDL_Sample.c)
# add_executable(${PROJECT_NAME} ${SOURCE})



add_custom_target(tar COMMAND
    ${CMAKE_COMMAND} -E tar "zcvf" "../source.tar.gz" 
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/lib"
    "${PROJECT_SOURCE_DIR}/res"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/COPYRIGHT.txt"
    "${PROJECT_SOURCE_DIR}/INSTALL.txt"
    "${PROJECT_SOURCE_DIR}/jsmn LICENSE"
    "${PROJECT_SOURCE_DIR}/README.txt"
    "${PROJECT_SOURCE_DIR}/SuperSimpleInstall.sh")

ADD_TEST(NAME test 
COMMAND "${EXECUTABLE_OUTPUT_PATH}/kchat-client"
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
ENABLE_TESTING()

add_custom_target(test-gui COMMAND
    "./bin/kchat-client" 
    "zuma.eecs.uci.edu"
    "11000"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(test-comm COMMAND
    "./bin/kchat-client" 
    "zuma.eecs.uci.edu"
    "11000"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})