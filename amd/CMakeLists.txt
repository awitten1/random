cmake_minimum_required(VERSION 3.24)

project(amduprof-play LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# For me, compile with:
# cmake -B build -DCMAKE_PREFIX_PATH=/opt/AMDuProf_5.0-1479/
find_file(AMD_POWER_PROFILE AMDTPowerProfileApi.h REQUIRED)
find_library(AMD_POWER_PROFILE_LIB AMDPowerProfileAPI PATH_SUFFIXES bin REQUIRED)

get_filename_component(AMD_U_PROF_DIR ${AMD_POWER_PROFILE} DIRECTORY)

add_executable(CollectCounters CollectAllCounters.cpp)
add_executable(LogTemperatures LogTemperatures.cpp enum_utils.cpp)

target_include_directories(CollectCounters PRIVATE ${AMD_U_PROF_DIR})
target_link_libraries(CollectCounters PRIVATE ${AMD_POWER_PROFILE_LIB})

target_include_directories(LogTemperatures PRIVATE ${AMD_U_PROF_DIR})
target_link_libraries(LogTemperatures PRIVATE ${AMD_POWER_PROFILE_LIB})


