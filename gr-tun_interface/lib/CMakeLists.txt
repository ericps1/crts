# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS} "${CMAKE_INSTALL_PREFIX}/build/")

list(APPEND tun_interface_sources
    t_source_impl.cc
    t_sink_impl.cc
	TUN.cpp
)

set(tun_interface_sources "${tun_interface_sources}" PARENT_SCOPE)
if(NOT tun_interface_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT tun_interface_sources)

add_library(gnuradio-tun_interface SHARED ${tun_interface_sources})
target_link_libraries(gnuradio-tun_interface ${Boost_LIBRARIES} ${tun_interface_libs})
set_target_properties(gnuradio-tun_interface PROPERTIES DEFINE_SYMBOL "gnuradio_tun_interface_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-tun_interface PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-tun_interface
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_tun_interface_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_tun_interface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_tun_interface.cc
)

add_executable(test-tun_interface ${test_tun_interface_sources})

target_link_libraries(
  test-tun_interface
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-tun_interface
)

GR_ADD_TEST(test_tun_interface test-tun_interface)
