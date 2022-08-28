# Based on cmake file from:
# https://github.com/mpizenberg/emscripten-opencv
cmake_minimum_required( VERSION 3.12 )
project( orb_wasm )

# Use C++ 11 by default
# Now use C++ 14, I think some of the PTAM code needs it
# adjusted back to 11 for ORB-SLAM as @carlodek, see below
set( CMAKE_CXX_STANDARD 11 )

set( opencvdir "/home/nick/src/opencv" )
set( orbdir "/home/nick/src/ORB_SLAM3" )
set( eigendir "/home/nick/src/eigen-3.3.7" )


# Needed for opencv2/opencv.hpp
include_directories( "${opencvdir}/include" )

# Needed by opencv.hpp for opencv2/opencv_modules.hpp
include_directories( "${opencvdir}//build_wasm" )

# Needed by opencv_modules.hpp for every module
file( GLOB opencv_include_modules "${opencvdir}/modules/*/include" )
include_directories( ${opencv_include_modules} "${orbdir}/include" ${orbdir} ${eigendir} "${orbdir}/include/CameraModels" )

add_compile_options("-pthread")
add_link_options("-pthread")

add_compile_options("-g")
add_compile_options("-fexceptions")
add_compile_options("SHELL: -s USE_ZLIB=1")
add_link_options("SHELL: -s SAFE_HEAP=1")
add_link_options("SHELL: -s PTHREAD_POOL_SIZE=8")
#add_link_options("-s LLD_REPORT_UNDEFINED")
add_link_options("-fexceptions")
#add_link_options("-s INITIAL_MEMORY=32000000")
add_link_options("SHELL: -s ALLOW_MEMORY_GROWTH=1")
add_link_options("-gsource-map")
#add_link_options("SHELL: -s INITIAL_MEMORY=134217728")
# the below is useful to debug pointer errors but doesn't seem to link properly
# https://emscripten.org/docs/debugging/Sanitizers.html 
#add_link_options("-fsanitize=address")
# add_link_options("--bind") # replaced by -lembind below
#add_compile_options("-O3")
#add_link_options("-s LLD_REPORT_UNDEFINED")

# from @carlodek https://github.com/raulmur/ORB_SLAM2/issues/264
add_link_options("SHELL: -s MAXIMUM_MEMORY=1GB")
add_link_options("SHELL: -s NO_EXIT_RUNTIME=1")
add_link_options("SHELL: -s ASSERTIONS=2")
add_link_options("SHELL: --preload-file assets")
add_link_options("--use-preload-plugins")
add_link_options("-lembind")
add_link_options("SHELL: --source-map-base http://localhost/sourcemap/")

# Our hello world executable
add_executable( orb_wasm orb_wasm.cpp )

# Link to opencv.js precompiled libraries
file( GLOB opencv_js "${opencvdir}/build_wasm/lib/*.a" )
set( orb_lib "${orbdir}/build/libORB_SLAM3.a" )
#file( GLOB_RECURSE orb_libs_external "${orbdir}/external/*.a" )
target_link_libraries(orb_wasm ${orb_lib} "${orbdir}/Thirdparty/g2o/lib/libg2o.a" "${orbdir}/Thirdparty/DBoW2/lib/libDBoW2.a" ${opencv_js} -lz)
