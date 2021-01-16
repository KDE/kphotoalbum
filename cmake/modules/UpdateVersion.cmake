# SPDX-FileCopyrightText: 2012-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause


if ( NOT DEFINED BASE_DIR )
	message ( FATAL_ERROR "UpdateVersion.cmake: BASE_DIR not set. Please supply base working directory!" )
endif()

# Step 1: query git if available:
if ( EXISTS "${BASE_DIR}/.git" )
	include ( "${CMAKE_CURRENT_LIST_DIR}/GitDescription.cmake" )
	git_get_description ( GIT_VERSION GIT_ARGS --dirty )

	# if both are set, check if project version and git version match
	if ( GIT_VERSION AND PROJECT_VERSION )
		string( FIND "${GIT_VERSION}" "${PROJECT_VERSION}" _position )

		# allow for position 0 or 1 (e.g. "5.0" in "v5.0")
		if ( ( _position LESS 0 ) OR ( _position GREATER 1 ) )
			message( AUTHOR_WARNING "Output of 'git describe' does not match PROJECT_VERSION! (${GIT_VERSION} vs ${PROJECT_VERSION})" )
		endif()
		unset(_position)
	endif()

	# git overrides project version
	if ( GIT_VERSION )
		set ( PROJECT_VERSION "${GIT_VERSION}" )
	endif()
endif()

# Step 2: configure version.h
if ( PROJECT_VERSION )
	# make sure we have the right variable set:
	set ( "${PROJECT_NAME}_VERSION" "${PROJECT_VERSION}" )

	message ( STATUS "Setting version information to ${PROJECT_VERSION}..." )
	if (NOT DEFINED OUTPUT_DIR)
		set(OUTPUT_DIR "${BASE_DIR}")
	endif()
	# write version info to a temporary file
	configure_file ( "${OUTPUT_DIR}/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h~" )
	# update info iff changed
	execute_process ( COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/version.h~" "${OUTPUT_DIR}/version.h" )
	# make sure info doesn't get stale
	file ( REMOVE "${CMAKE_CURRENT_BINARY_DIR}/version.h~" )
else()
	# if we got no version, but we have a version.h, don't complain:
	if ( NOT EXISTS "${OUTPUT_DIR}/version.h" )
		message ( SEND_ERROR "The generated file 'version.h' does not exist!" )
		message ( AUTHOR_WARNING "When creating a release tarball, please make sure to run cmake -P ${CMAKE_CURRENT_LIST_FILE}" )
	endif()
endif()
