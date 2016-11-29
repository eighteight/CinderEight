cmake_minimum_required(VERSION 2.8)
PROJECT(Cinder)
SET(CMAKE_BUILD_TYPE release)
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
#Boost
#SET(BOOST_ROOT "d:/boost_1_48_0")
#FIND_PACKAGE(Boost )
#if(Boost_FOUND)
#
#    message("boost:  " ${Boost_INCLUDE_DIRS})
#    message("boost:  " ${Boost_LIBRARY_DIRS})
#
#    include_directories(${Boost_INCLUDE_DIRS})
#    link_directories(${Boost_LIBRARY_DIRS} )
#endif()
#
#set(Boost_USE_STATIC_RUNTIME ON)

# set cmake to use static runtime.
foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
   if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "/MD")
endforeach(flag_var)

#incude cinder and boost header files and libraries.
set(CINDER_ROOT $ENV{CINDER_DIR} )
include_directories("${CINDER_ROOT}/include" "${CINDER_ROOT}/boost")
link_directories("${CINDER_ROOT}/lib" "${CINDER_ROOT}/lib/msw")


# the Cinder program source code.
INCLUDE_DIRECTORIES("include")
ADD_EXECUTABLE(Cinder WIN32
 "Resources.rc"
 "src/AudioEngineApp.cpp"
)


set(VS_IGNORE_LIBRARY_FLAGS "/NODEFAULTLIB:msvcprt.lib /NODEFAULTLIB:msvcrt.lib")
if(MSVC)
    SET_TARGET_PROPERTIES(Cinder
                        PROPERTIES LINK_FLAGS   {VS_IGNORE_LIBRARY_FLAGS})
    SET_TARGET_PROPERTIES(Cinder PROPERTIES ENABLE_EXPORTS true)
endif()


TARGET_LINK_LIBRARIES(Cinder cinder)