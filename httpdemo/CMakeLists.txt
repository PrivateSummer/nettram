cmake_minimum_required(VERSION 3.0)
get_filename_component(filepath ../Comm.in ABSOLUTE)
string(REPLACE /Comm.in "" PRO_ROOT ${filepath})
include(${PRO_ROOT}/Comm.in)

#头文件目录
include_directories(${PRO_ROOT}/src/nt)
include_directories(${PRO_ROOT}/comm/src)
include_directories(src)
include_directories(src/service)
include_directories(src/logic)

# 递归查找目录下的所有源文件
# 并将名称保存到 SRC_FILES 变量
file(GLOB_RECURSE SRC_FILES src/*.cpp)

find_package(Threads)

#生成可执行程序
add_executable(httpdemo ${SRC_FILES})

#依赖库
target_link_libraries(httpdemo "-Xlinker \"-(\"")
target_link_libraries(httpdemo ${PRO_ROOT}/src/libnt_http.a)
target_link_libraries(httpdemo ${PRO_ROOT}/comm/libcommon_comm.a)
target_link_libraries(httpdemo ${PRO_ROOT}/ntlib/lib/libntlib.a)
target_link_libraries(httpdemo "-Xlinker \"-)\"")
target_link_libraries(httpdemo libevent.a)
IF(WITH_BOOST)
target_link_libraries(httpdemo ${BOOST_LIB_DIR}/libboost_system.a)
target_link_libraries(httpdemo ${BOOST_LIB_DIR}/libboost_thread.a)
ENDIF(WITH_BOOST)
target_link_libraries(httpdemo ${CMAKE_THREAD_LIBS_INIT})

#target_link_libraries(httpdemo pthread)
target_link_libraries(httpdemo rt)


