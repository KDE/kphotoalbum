# SPDX-FileCopyrightText: 2012-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

# query_git_version uses git-describe to extract version information from git and sets the variable named ${OUTPUT_VARIABLE} accordingly.
# If BASE_DIR is not a git directory, OUTPUT_VARIABLE is not set.
function(query_git_version BASE_DIR OUTPUT_VARIABLE)
    if(EXISTS "${BASE_DIR}/.git")
        include("${CMAKE_CURRENT_LIST_DIR}/GitDescription.cmake")
        # shallow cloning in gitlab CI requires --always parameter to not fail due to missing tag information:
        git_get_description(GIT_VERSION GIT_ARGS --dirty --always)
        if(GIT_VERSION)
            set(${OUTPUT_VARIABLE} "${GIT_VERSION}" PARENT_SCOPE)
        endif()
    endif()
endfunction()



# set_version_variable sets OUTPUT_VARIABLE to GIT_VERSION if that is usable.
# GIT_VERSION is considered usable if starts with the scheme "v<MAJOR>.<MINOR>".
# If GIT_VERSION starts with a "g", it is considered a commit hash and appended to the FALLBACK_VERSION.
# GIT_VERSION examples:
#  "v5.8" -> tagged commit
#  "v5.8.1" -> tagged commit
#  "v5.8.1-160-g4fb259da" -> commit 160 after the last tagged version
#  "g4fb259da" -> no last recent tag available(usually caused by shallow cloning in Gitlab CI)
#  "v5.8.1-160-g4fb259da-dirty" -> commit with local changes
#  "v5.8.1-dirty" tagged commit with local changes
# PROJECT_VERSION example: "5.8.1"
function(set_version_variable OUTPUT_VARIABLE GIT_VERSION FALLBACK_VERSION)
    if("${GIT_VERSION}" STREQUAL "")
        set("${OUTPUT_VARIABLE}" "${FALLBACK_VERSION}" PARENT_SCOPE)
    elseif("${GIT_VERSION}" MATCHES "^v([0-9]+[.][0-9]+([.][0-9]+)?)")
        if(NOT "${FALLBACK_VERSION}" STREQUAL "")
            # if both are set, check if project version and git version match
            if(NOT "${CMAKE_MATCH_1}" VERSION_EQUAL "${FALLBACK_VERSION}")
                message(AUTHOR_WARNING "Git version does not match PROJECT_VERSION!(${GIT_VERSION} vs ${FALLBACK_VERSION})")
            endif()
        endif()
        set("${OUTPUT_VARIABLE}" "${GIT_VERSION}" PARENT_SCOPE)
    elseif("${GIT_VERSION}" MATCHES "^g")
        message(STATUS "Git version only contains the hash and no tag info. Falling back to project version...")
        set("${OUTPUT_VARIABLE}" "v${FALLBACK_VERSION}-${GIT_VERSION}" PARENT_SCOPE)
    else()
        message(AUTHOR_WARNING "Unexpected input in git version! Please file a bug!(${GIT_VERSION})")
    endif()
endfunction()

# for unit tests, run cmake -D UPDATEVERSION_RUN_UNITTESTS=true -P UpdateVersion.cmake
if(UPDATEVERSION_RUN_UNITTESTS)
    function(expect value expected_value)
        if(NOT "${value}" STREQUAL "${expected_value}")
            message(ERROR " Expected value: ${expected_value}, actual value: ${value}")
        endif()
    endfunction()
    set_version_variable(output "v5.8.0" "5.8")
    expect("${output}" "v5.8.0")
    set_version_variable(output "v5.8" "5.8")
    expect("${output}" "v5.8")
    set_version_variable(output "v5.8" "5.8.0")
    expect("${output}" "v5.8")
    set_version_variable(output "v5.8.1" "5.8.1")
    expect("${output}" "v5.8.1")
    set_version_variable(output "v5.8.1-160-g4fb259da" "5.8.1")
    expect("${output}" "v5.8.1-160-g4fb259da")
    set_version_variable(output "g4fb259da" "5.8")
    expect("${output}" "v5.8-g4fb259da")
    set_version_variable(output "v5.8.1-dirty" "5.8.1")
    expect("${output}" "v5.8.1-dirty")
    set_version_variable(output "v5.8.1-160-g4fb259da-dirty" "5.8.1")
    expect("${output}" "v5.8.1-160-g4fb259da-dirty")
    return()
endif()



if(NOT DEFINED BASE_DIR)
    message(FATAL_ERROR "UpdateVersion.cmake: BASE_DIR not set. Please supply base working directory!")
endif()

# Step 1: query git if available:
query_git_version("${BASE_DIR}" GIT_VERSION)
set_version_variable(PROJECT_VERSION "${GIT_VERSION}" "${PROJECT_VERSION}")

# Step 2: configure version.h
if(PROJECT_VERSION)
    # make sure we have the right variable set:
    set("${PROJECT_NAME}_VERSION" "${PROJECT_VERSION}")

    message(STATUS "Setting version information to ${PROJECT_VERSION}...")
    if(NOT DEFINED OUTPUT_DIR)
        set(OUTPUT_DIR "${BASE_DIR}")
    endif()
    # write version info to a temporary file
    configure_file("${OUTPUT_DIR}/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h~")
    # update info iff changed
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/version.h~" "${OUTPUT_DIR}/version.h")
    # make sure info doesn't get stale
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/version.h~")
else()
    # if we got no version, but we have a version.h, don't complain:
    if(NOT EXISTS "${OUTPUT_DIR}/version.h")
        message(SEND_ERROR "The generated file 'version.h' does not exist!")
        message(AUTHOR_WARNING "When creating a release tarball, please make sure to run cmake -P ${CMAKE_CURRENT_LIST_FILE}")
    endif()
endif()
# vi:expandtab:tabstop=4 shiftwidth=4:
