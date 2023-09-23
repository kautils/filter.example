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

set(module_name subtract)
unset(srcs)
file(GLOB srcs ${CMAKE_CURRENT_LIST_DIR}/*.cc)
set(${module_name}_common_pref
    MODULE_PREFIX kautil filter althmetic
    MODULE_NAME ${module_name}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include> 
    SOURCES ${srcs}
    EXPORT_NAME_PREFIX ${PROJECT_NAME}
    EXPORT_VERSION ${PROJECT_VERSION}
    EXPORT_VERSION_COMPATIBILITY AnyNewerVersion
    DESTINATION_INCLUDE_DIR include/kautil
    DESTINATION_CMAKE_DIR cmake
    DESTINATION_LIB_DIR lib
)


list(APPEND ${m}_unsetter ${m}_thread_cnt)

include(ProcessorCount)
ProcessorCount(${m}_thread_cnt)
CMakeFetchKautilModule(c11_string_allocator
        GIT https://github.com/kautils/btree_search.git
        REMOTE origin 
        TAG v0.0.1
        CMAKE_BUILD_OPTION -j ${${m}_thread_cnt}
        FORCE_UPDATE
        )

find_package(KautilAlgorithmBtreeSearch.0.0.1.static REQUIRED)

CMakeGitCurrentCommitHash(${m}_filter_id)
string(SUBSTRING "${${m}_filter_id}" 0 7 ${m}_filter_short_id)
set(${m}_filter_hr_id ${PROJECT_NAME}/${${m}_filter_short_id}) # hr_id : human readable id
CMakeLibraryTemplate(${module_name} EXPORT_LIB_TYPE shared ${${module_name}_common_pref} )
#CMakeLibraryTemplate(${module_name} EXPORT_LIB_TYPE static ${${module_name}_common_pref} )


set(__t ${${module_name}_shared_tmain})
add_executable(${__t})
target_sources(${__t} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/unit_test.cc)
target_link_libraries(${__t} PRIVATE ${${module_name}_shared} kautil::argorithm::btree_search::0.0.1::static)
target_compile_definitions(${__t} PRIVATE ${${module_name}_shared_tmain_ppcs})

foreach(__v ${${m}_unsetter})
    unset(${__v})
endforeach()
unset(${m}_unsetter)
set(m ${${PROJECT_NAME}_m_evacu})

return()
