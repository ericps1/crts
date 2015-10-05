INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_TUN_BLOCKS tun_blocks)

FIND_PATH(
    TUN_BLOCKS_INCLUDE_DIRS
    NAMES tun_blocks/api.h
    HINTS $ENV{TUN_BLOCKS_DIR}/include
        ${PC_TUN_BLOCKS_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    TUN_BLOCKS_LIBRARIES
    NAMES gnuradio-tun_blocks
    HINTS $ENV{TUN_BLOCKS_DIR}/lib
        ${PC_TUN_BLOCKS_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TUN_BLOCKS DEFAULT_MSG TUN_BLOCKS_LIBRARIES TUN_BLOCKS_INCLUDE_DIRS)
MARK_AS_ADVANCED(TUN_BLOCKS_LIBRARIES TUN_BLOCKS_INCLUDE_DIRS)

