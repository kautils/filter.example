

set(${PROJECT_NAME}_m_evacu ${m})
set(m CMakePullLocalRepositoryAsSymLink)
list(APPEND ${m}_unsetter )


if(NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeKautilHeader.cmake)
    file(DOWNLOAD https://raw.githubusercontent.com/kautils/CMakeKautilHeader/v0.0.1/CMakeKautilHeader.cmake ${CMAKE_BINARY_DIR}/CMakeKautilHeader.cmake)
endif()
include(${CMAKE_BINARY_DIR}/CMakeKautilHeader.cmake)
git_clone(https://raw.githubusercontent.com/kautils/CMakeLibrarytemplate/v0.0.1/CMakeLibrarytemplate.cmake)
git_clone(https://raw.githubusercontent.com/kautils/CMakeGitCurrentCommitHash/v0.0.1/CMakeGitCurrentCommitHash.cmake)
git_clone(https://raw.githubusercontent.com/kautils/CMakeFetchKautilModule/v0.0.1/CMakeFetchKautilModule.cmake)


include(ProcessorCount)
ProcessorCount(${m}_thread_cnt)




### start temporal
set(__flow kautil_flow_0.0.1_static)
### end temporal



set(module_name example)
unset(srcs)
file(GLOB srcs ${CMAKE_CURRENT_LIST_DIR}/*.cc)
set(${module_name}_common_pref
    MODULE_PREFIX kautil filter
    MODULE_NAME ${module_name}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include> 
    SOURCES ${srcs}
    LINK_LIBS ${__flow} 
    EXPORT_NAME_PREFIX ${PROJECT_NAME}
    EXPORT_VERSION ${PROJECT_VERSION}
    EXPORT_VERSION_COMPATIBILITY AnyNewerVersion
    DESTINATION_INCLUDE_DIR include/kautil
    DESTINATION_CMAKE_DIR cmake
    DESTINATION_LIB_DIR lib
)


set(CMAKE_VERBOSE_MAKEFILE TRUE)

list(APPEND ${m}_unsetter ${m}_thread_cnt)
CMakeGitCurrentCommitHash(${m}_filter_id)
string(SUBSTRING "${${m}_filter_id}" 0 7 ${m}_filter_short_id)
set(${m}_filter_hr_id ${PROJECT_NAME}/${${m}_filter_short_id}) # hr_id : human readable id
CMakeLibraryTemplate(${module_name} EXPORT_LIB_TYPE shared ${${module_name}_common_pref} )
target_compile_definitions(${${module_name}_shared} PUBLIC
    -DFILTER_ID_SHORT="${${m}_filter_short_id}"
    -DFILTER_ID_HR="${${m}_filter_hr_id}"
    -DFILTER_ID="${${m}_filter_id}"
)





set(__t ${${module_name}_shared_tmain})
add_executable(${__t})
target_sources(${__t} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/unit_test.cc)
target_link_libraries(${__t} PRIVATE ${${module_name}_shared} ${__flow})
target_compile_definitions(${__t} PRIVATE ${${module_name}_shared_tmain_ppcs})


foreach(__v ${${m}_unsetter})
    unset(${__v})
endforeach()
unset(${m}_unsetter)
set(m ${${PROJECT_NAME}_m_evacu})

