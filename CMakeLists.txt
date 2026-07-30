# ---------------------------------------------------- #
#                 Required envs assert                 #
# ---------------------------------------------------- #

if(NOT DEFINED ENV{COMMONLIB_SSE_FOLDER})
  message(FATAL_ERROR "Missing COMMONLIB_SSE_FOLDER environment variable")
endif()

# ---------------------------------------------------- #
#               Imports and definitions                #
# ---------------------------------------------------- #

include(cmake/lib/copyOutputs.cmake)
include(cmake/lib/automaticGameFolderOutput.cmake)
include(cmake/lib/autoCollectSources.cmake)

add_src_folder(src)
add_include_folder(include)

add_src_folder(vendor/src)
add_include_folder(vendor/include)


cmake_minimum_required(VERSION 3.21)
set(AUTHOR_NAME "SkyrimThiago")
set(PRODUCT_NAME "SkyPlace")
set(BEAUTIFUL_NAME "SkyPlace")

add_compile_definitions(IS_SKY_PLACE=true)

project(${PRODUCT_NAME} VERSION 0.1.12.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# ---------------------------------------------------- #
#                      Resources                       #
# ---------------------------------------------------- #

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY)

set(RESOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
set(RESOURCE_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.res)

add_custom_command(
    OUTPUT ${RESOURCE_OUTPUT}
    COMMAND rc /fo ${RESOURCE_OUTPUT} ${RESOURCE_FILE}
    DEPENDS ${RESOURCE_FILE}
)

add_custom_target(Resource ALL DEPENDS ${RESOURCE_OUTPUT})

# ---------------------------------------------------- #
#                Plugin Configuration                  #
# ---------------------------------------------------- #

include(FetchContent)

set(ENABLE_SKYRIM_SE ON CACHE BOOL " " FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL " " FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL " " FORCE)
set(BUILD_TESTS OFF CACHE BOOL " " FORCE)

add_subdirectory("$ENV{COMMONLIB_SSE_FOLDER}" ${CMAKE_CURRENT_BINARY_DIR}/_deps/clib-build)
get_target_property(COMMONLIB_SRC_DIR CommonLibSSE SOURCE_DIR)

file(
  GLOB_RECURSE
  sources
  ${CMAKE_CURRENT_BINARY_DIR}/version.rc
  ${sources}
)

source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  FILES ${headers} ${sources}
)

set(BUILD_TESTS OFF)
include(${COMMONLIB_SRC_DIR}/cmake/CommonLibSSE.cmake)

add_commonlibsse_plugin(
  ${PROJECT_NAME}
  AUTHOR AUTHOR_NAME
  SOURCES ${headers} ${sources}
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) 
target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h)
target_compile_definitions(${PROJECT_NAME}  PRIVATE BEAUTIFUL_NAME="${BEAUTIFUL_NAME}" IMGUI_DEFINE_MATH_OPERATORS GLM_ENABLE_EXPERIMENTAL)


set(header_dirs "")

foreach(header ${headers})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND header_dirs ${dir})
endforeach()

list(REMOVE_DUPLICATES header_dirs)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
    ${header_dirs}
)

find_package(imgui CONFIG REQUIRED)
find_package(NanoSVG REQUIRED)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)
find_library(DIRECTXTK_LIBRARY NAMES DirectXTK PATHS ${CMAKE_BINARY_DIR}/debug/vcpkg_installed/x64-windows-static/lib)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		imgui::imgui
         NanoSVG::nanosvg
         NanoSVG::nanosvgrast
         ${DIRECTXTK_LIBRARY}
         ${DETOURS_LIBRARY}
)


# ---------------------------------------------------- #
#                        Output                        #
# ---------------------------------------------------- #

set(wildlander_output false)
set(steam_owrt_output false)
set(steam_mods_output true)
automaticGameFolderOutput(wildlander_output, steam_owrt_output, steam_mods_output)
