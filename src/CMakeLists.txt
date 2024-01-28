cmake_minimum_required(VERSION 3.0)
project(fsinodes)

set(CMAKE_CXX_STANDARD 20)

add_executable(fsinodes
        main.cpp

        Simulator.cpp
        util/FSCmdParser.cpp

        fs/FileSystem.cpp

        fs/container/MMappedFile.cpp

        fs/sections/superblock/Superblock.cpp

        fs/sections/bitmaps/Bitmap.cpp

        fs/sections/inodes/Inodes.cpp
        fs/sections/inodes/Inode.cpp

        fs/sections/data/Data.cpp
        fs/sections/data/DataBlock.cpp

        fs/sections/data/util/Iterator_DataBlocks.cpp
        fs/sections/data/util/Iterator_DirItems.cpp
)
