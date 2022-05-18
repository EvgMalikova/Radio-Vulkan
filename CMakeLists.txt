cmake_minimum_required(VERSION 3.8)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#enable_language(Fortran)



option(BUILD_GUI "Build interactive" ON)
option(BUILD_MPI "Enable MPI support" ON)
option(BUILD_NVVK "Enable NVVK support" OFF)

option(BUILD_DOC "Build documentation" ON)

set(LOGGING "All" CACHE STRING "Enable Log output")
set_property(CACHE LOGGING PROPERTY STRINGS All Debug Info Warning Error Fatal None)
add_definitions( -DLOGGING=${LOGGING} )
configure_file( config.h.in ${CMAKE_BINARY_DIR}/config.h)
#file(GLOB SHADERS "shaders/*")

set (SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/21_shader_ubo.frag
             ${CMAKE_CURRENT_SOURCE_DIR}/shaders/21_shader_ubo.vert
             ${CMAKE_CURRENT_SOURCE_DIR}/shaders/26_shader_textures.vert
             ${CMAKE_CURRENT_SOURCE_DIR}/shaders/26_shader_textures.frag
             
)

if(BUILD_DOC)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DLOG_OUTPUT")

find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
    
    
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# ImGui
set(IMGUI_DIR imgui/)
include_directories(${IMGUI_DIR} ..)


endif ()

find_package(libjpeg-turbo)
find_library(LIBJPEGTURBO_LIBRARY turbojpeg PATHS ${LIBJPEGTURBO_ROOT_DIR} PATH_SUFFIXES lib lib64)

if(LIBJPEGTURBO_FOUND)
set(LIBJPEGTURBO_INCLUDE_DIRS ${LIBJPEGTURBO_INCLUDE_DIR})
set(LIBJPEGTURBO_LIBRARIES ${LIBJPEGTURBO_LIBRARY})
message(STATUS "Found mysqlcppconn    (include: ${LIBJPEGTURBO_INCLUDE_DIR}, library: ${LIBJPEGTURBO_LIBRARY})")
  mark_as_advanced(LIBJPEGTURBO_ROOT_DIR LIBJPEGTURBO_LIBRARY_RELEASE LIBJPEGTURBO_LIBRARY_DEBUG
                                 LIBJPEGTURBO_LIBRARY LIBJPEGTURBO_INCLUDE_DIR)
endif()


find_package(libwebsockets CONFIG REQUIRED)


# Third party library dependencies
#
set(SOURCE_FILES interactive.cpp sampleApp.cpp window.cpp camera.cpp renderer.cpp context.cpp pipeline.cpp helpers.cpp loaders.cpp postprocessing.cpp server/controller.cc server/camera.cc )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/render_doc)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/c_utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tclap)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/third_party/imgui)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/third_party/stb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/third_party/tinyobjloader)

set(SPLOTCH_SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/transform.cxx
   ${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/error_handling.cc
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/mpi_support.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/paramfile.cc 
${CMAKE_CURRENT_SOURCE_DIR}/reader/gadget_reader.cc 
${CMAKE_CURRENT_SOURCE_DIR}/fitsReader.cxx 
${CMAKE_CURRENT_SOURCE_DIR}/reader/bin_reader.cc
${CMAKE_CURRENT_SOURCE_DIR}/reader/bin_writer.cc 
${CMAKE_CURRENT_SOURCE_DIR}/splotch/splotchutils.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/splotch.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/scenemaker.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/splotch_host.cc 
#${CMAKE_CURRENT_SOURCE_DIR}/splotch/new_renderer.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/walltimer.cc 
${CMAKE_CURRENT_SOURCE_DIR}/c_utils/walltime_c.c
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/string_utils.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/announce.cc 
${CMAKE_CURRENT_SOURCE_DIR}/cxxsupport/ls_image.cc 


)
set(HELPER_SOURCE
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/context_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/error_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/extensions_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/debug_util_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvh/nvprint.cpp


${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/resourceallocator_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/memallocator_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/stagingmemorymanager_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/buffersuballocator_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/memorymanagement_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/memallocator_dedicated_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/samplers_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/images_vk.cpp
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/descriptorsets_vk.cpp

${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/raytraceKHR_vk.cpp #uses nvmath
${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/commands_vk.cpp

#${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/third_party/tinyobjloader/tyny_obj_loader.cc)
)
#file(GLOB NVHELPERSVK_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/*.inl ${CMAKE_CURRENT_SOURCE_DIR}/nvhelpers/nvvk/*.h)

#   list(REMOVE_ITEM NVHELPERSVK_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvvk/memallocator_vma_vk.hpp)
#   list(REMOVE_ITEM NVHELPERSVK_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvvk/memallocator_vma_vk.inl)
 #  list(REMOVE_ITEM NVHELPERSVK_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvvk/appbase_vk.hpp)
#   list(REMOVE_ITEM NVHELPERSVK_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvvk/appbase_vk.cpp)
 
# source_group("nvvk" FILES ${NVHELPERSVK_SOURCE})
#file(GLOB NVHELPERS_SOURCE nvh/*.cpp nvh/*.hpp nvh/*.inl nvh/*.h)


#  list(REMOVE_ITEM NVHELPERS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvh/gltfscene.cpp)
#  list(REMOVE_ITEM NVHELPERS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvh/gltfscene.hpp)
#  list(REMOVE_ITEM FILEFORMATS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/fileformats/tiny_gltf_freeimage.h)

file(GLOB NVMATH_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nvmath/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/nvmath/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/nvmath/*.inl ${CMAKE_CURRENT_SOURCE_DIR}/nvmath/*.h)

  find_library(
    cfitsio 
    cfitsio
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/lib64)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/WSRTI/tjpp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/WSRTI/websocketplus/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/WSRTI/serializer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/WSRTI/syncqueue)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/lib64)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/INSTALL/lib64)
include_directories(/group/director2185/emalikova/particle-based-vis/INSTALL/include)


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SPLOTCH_SOURCE} ${HELPER_SOURCE} ${NVMATH_SOURCE})


if(BUILD_GUI)
  find_package(glfw3 3.3 REQUIRED)
  add_definitions(-DUSE_GLFW)
endif()

if(BUILD_MPI)
find_package(MPI REQUIRED)
  add_definitions(-DUSE_MPIRV)
add_definitions(-DOMPI_SKIP_MPICXX)
endif()

if(BUILD_NVVK)
  add_definitions(-DUSE_NVVK)
endif()

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
  message(STATUS "VULKAN found")
  include_directories(${Vulkan_INCLUDE_DIR})
  link_libraries (${Vulkan_LIBRARY})
endif (Vulkan_FOUND)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY})# ${MPI_C_LIBRARIES})
#MPI::MPI_CXX)
if(BUILD_MPI)
target_link_libraries(${PROJECT_NAME} ${MPI_C_LIBRARIES})
endif()
target_link_libraries(${PROJECT_NAME} ${cfitsio} )
target_link_libraries(${PROJECT_NAME} ${LIBJPEGTURBO_LIBRARY} )
if(BUILD_GUI)

 target_link_libraries(${PROJECT_NAME} glfw )
endif()

# These are additional include paths you will need
foreach(item "${LIBWEBSOCKETS_INCLUDE_DIRS}")
	include_directories(${item})
	set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}" ${item})
endforeach()

	if (websockets_shared)
		target_link_libraries(${PROJECT_NAME}
					websockets_shared
					${LIBWEBSOCKETS_DEP_LIBS})
		add_dependencies(${PROJECT_NAME}
					websockets_shared)
	else()
		target_link_libraries(${PROJECT_NAME}
					websockets
					${LIBWEBSOCKETS_DEP_LIBS})
	endif()
	
	if (HAS_LWS_WITH_SECURE_STREAMS_PROXY_API OR has_ss_proxy OR
	    LWS_WITH_SECURE_STREAMS_PROXY_API)

		add_compile_options(-DLWS_SS_USE_SSPC)
		add_executable(${PROJECT_NAME}-client ${SRCS})

		if (websockets_shared)
			target_link_libraries(${PROJECT_NAME}-client
						websockets_shared
						${LIBWEBSOCKETS_DEP_LIBS})
			add_dependencies(${PROJECT_NAME}-client
						websockets_shared)
		else()
			target_link_libraries(${PROJECT_NAME}-client
						websockets
						${LIBWEBSOCKETS_DEP_LIBS})
		endif()
	endif()



file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders)

foreach(SHADER ${SHADERS})
	get_filename_component(SHADER_NAME ${SHADER} NAME)

	add_custom_command(
		OUTPUT shaders/${SHADER_NAME}.spv
		COMMAND glslc ${SHADER}  -o shaders/${SHADER_NAME}.spv #v--target-env=vulkan1.2
		DEPENDS ${SHADER}
	)
	target_sources(${PROJECT_NAME} PRIVATE shaders/${SHADER_NAME}.spv)
endforeach()

#Start testing

#enable_testing()

#set(MPITEST_FILES mpi_test.cpp camera.cpp renderer.cpp context.cpp pipeline.cpp helpers.cpp loaders.cpp postprocessing.cpp )

#add_definitions(-DUSE_MPIRV)
#add_definitions(-DOMPI_SKIP_MPICXX)
#remove_definitions(-DUSE_GLFW)

#add_executable(mpi_test  ${MPITEST_FILES} ${SPLOTCH_SOURCE} )
#target_link_libraries(mpi_test ${Vulkan_LIBRARY})# ${MPI_C_LIBRARIES})
#target_link_libraries(mpi_test ${MPI_C_LIBRARIES})
#target_link_libraries(mpi_test  ${cfitsio} )

#add_test(MPITest mpi_test)