# cmake usage tips

# run cmake 
# 1. if build/ folder not exist, mkdir build
# 2. cd build/
# 3. cmake ..
# 4. make

# rerun cmake after modifications on CMakeLists.txt
# 1. cd build/
# 2. make clean
# 3. cmake ..
# 4. make


enable_testing()

#specify minimum required cmake version
cmake_minimum_required(VERSION 3.10)


#set compiler
#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
add_compile_options("-Qunused-arguments")



#set project name
project(TritonBot)

#set target name
set(Target TritonBot.exe)


#set c++ version
set(CMAKE_CXX_STANDARD 17)

#set build type: Possible values are empty, Debug, Release, RelWithDebInfo and MinSizeRel
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "-Wall") #enable all warnings, which can be annoying sometimes, mute it by comment out this line
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3") #optimization level: O1(lowest), O2, or O3(highest)

#something good to have
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


#find dependent packages 
#minimum version 1.65.1 (default for Ubuntu 18.04)
find_package(Boost 1.65.1 REQUIRED COMPONENTS date_time chrono system thread log)
#armadillo math library
find_package(Armadillo REQUIRED)
#google protobuf library
find_package(Protobuf REQUIRED)


#set include directories
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ARMADILLO_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/proto/)




#set variable to represent source files in the source folders
# - make sure to rerun cmake everytime a file is added to or remnove from the folders
aux_source_directory(./source Main_srcs)
aux_source_directory(source/CoreModules/ControlModule Control_srcs)
aux_source_directory(source/Misc/Utility Utility_srcs)
aux_source_directory(source/PeriphModules/FirmClientModule MCInterface_srcs)
aux_source_directory(source/CoreModules/EKF-Module EKF_srcs)
aux_source_directory(./source/Config Config_srcs)
aux_source_directory(source/CoreModules/MotionModule Motion_srcs)
aux_source_directory(source/PeriphModules/RemoteServers SERV_srcs)
aux_source_directory(source/CoreModules/BallCaptureModule BCAP_srcs)






#add macro definitions "#define xxx"
add_definitions(-DBOOST_LOG_DYN_LINK) #macro needed by boost logger





####--------------------------------Protobuf--------------------------------###
##run protoc to auto generate protobuf source code
file(GLOB PROTO_DEFS "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
set(PROTO_GEN_MSG  "${CMAKE_CURRENT_SOURCE_DIR}/proto/proto_generated.cmake_msg")
#add_custom_command( OUTPUT ${PROTO_GEN_MSG}
#                    COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated
#                    COMMAND protoc  -I=${CMAKE_CURRENT_SOURCE_DIR}/proto/
#                                    --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated
#                                    ${PROTO_DEFS}
#                    COMMAND touch ${PROTO_GEN_MSG}
#                    DEPENDS ${PROTO_DEFS}
#                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
#                    COMMENT "Generating Protobuf Source Code in ${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated"
#                    VERBATIM
#                   )
#
#add_custom_target(PROTO_GEN DEPENDS ${PROTO_GEN_MSG})
file(GLOB PROTO_PRIVATE_INC "${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated/*.pb.h")
file(GLOB PROTO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated/*.pb.cc")
####------------------------------------------------------------------------###



#add target to be built
add_executable(${Target} ${Main_srcs} ${Control_srcs} ${Utility_srcs} ${PROTO_PRIVATE_INC} ${PROTO_SRC}
               ${MCInterface_srcs} ${EKF_srcs} ${Config_srcs} ${Motion_srcs} ${SERV_srcs} ${BCAP_srcs})

## need to generate proto sources before compiling everything else
#add_dependencies(${Target} PROTO_GEN)

#specify the libraries to be linked to the target added by [add_executable]
target_link_libraries(${Target} PUBLIC  ${Boost_Libraries} Boost::date_time 
                                                           Boost::chrono 
                                                           Boost::system 
                                                           Boost::thread 
                                                           Boost::log)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${Target} PUBLIC  /usr/local/lib/libarmadillo.dylib)
else() # Ubuntu
    #${Armadillo_Libraries} # this won't compile matrix part of the library, strangely 
    #-larmadillo # use this old-school way to link the lib
    target_link_libraries(${Target} PUBLIC  -larmadillo)
endif()

target_link_libraries(${Target} PUBLIC  ${PROTOBUF_LIBRARIES})



#add a custom clean target to clean autogenerated source code of protobuf, 
#   you usually don't need to clean them unless the source code is corrupted 
add_custom_target(clean-proto-generated
    COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated
    COMMAND rm -f ${CMAKE_CURRENT_SOURCE_DIR}/proto/proto_generated.cmake_msg  
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Removed Auto Generated Protobuf Source Code"
    VERBATIM
)

add_custom_target(proto
    COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated
    COMMAND protoc  -I=${CMAKE_CURRENT_SOURCE_DIR}/proto/
    --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated
    ${PROTO_DEFS}
    COMMAND touch ${PROTO_GEN_MSG}
    DEPENDS ${PROTO_DEFS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating Protobuf Source Code in ${CMAKE_CURRENT_SOURCE_DIR}/proto/ProtoGenerated"
    VERBATIM)

#Tests

#add_subdirectory(googletest)
#add_subdirectory(test)
