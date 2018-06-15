# CMake version 
cmake_minimum_required (VERSION 3.5)

#project name
PROJECT(kiwistrmd)

#head file path
INCLUDE_DIRECTORIES(src/studease.cn)
INCLUDE_DIRECTORIES(src/studease.cn/core)
INCLUDE_DIRECTORIES(src/studease.cn/event)
INCLUDE_DIRECTORIES(src/studease.cn/utils)
INCLUDE_DIRECTORIES(src/studease.cn/http)
INCLUDE_DIRECTORIES(src/studease.cn/websocket)
INCLUDE_DIRECTORIES(src/kiwistrmd.com)
INCLUDE_DIRECTORIES(src/kiwistrmd.com/core)

#add sub directory
ADD_SUBDIRECTORY(src/studease.cn)
ADD_SUBDIRECTORY(src/kiwistrmd.com)

#source directory
AUX_SOURCE_DIRECTORY(src DIR_SRC)

#add executable file
ADD_EXECUTABLE(kiwistrmd ${DIR_SRC})

#add link library
TARGET_LINK_LIBRARIES(kiwistrmd ksd)
TARGET_LINK_LIBRARIES(kiwistrmd websocket)
TARGET_LINK_LIBRARIES(kiwistrmd http)
TARGET_LINK_LIBRARIES(kiwistrmd utils)
TARGET_LINK_LIBRARIES(kiwistrmd event)
TARGET_LINK_LIBRARIES(kiwistrmd core)

TARGET_LINK_LIBRARIES(kiwistrmd pthread)
TARGET_LINK_LIBRARIES(kiwistrmd m)
TARGET_LINK_LIBRARIES(kiwistrmd crypto)