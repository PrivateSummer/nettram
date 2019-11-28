cmake_minimum_required(VERSION 3.0)
set(PRO_ROOT "..")
include(${PRO_ROOT}/Comm.in)

#头文件目录


# 查找目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(./nt DIR_LIB_SRCS)
# 生成链接库
add_library (nt_http ${DIR_LIB_SRCS})

