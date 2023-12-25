cmake_minimum_required(VERSION 3.0)
project(fsinodes)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(fsinodes main.cpp
        Simulator.cpp
        fs/FileSystem.cpp
        util/FSCmdParser.cpp
        io/MMappedFile.cpp
        fs/components/Superblock.cpp
        fs/components/util/Bitmap.cpp
        fs/components/Inodes.cpp
        fs/components/Inode.cpp
        fs/components/Data.cpp
        fs/components/DataBlock.cpp
        fs/components/util/Iterator_DataBlocks.cpp
        fs/components/util/Iterator_DirItems.cpp
)
