# git or tarball?
if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git )
	# -> git:
	include ( "${CMAKE_CURRENT_LIST_DIR}/GitDescription.cmake" )

	git_get_description ( KPA_VERSION GIT_ARGS --dirty )
	if ( NOT KPA_VERSION )
		set ( KPA_VERSION "unknown" )
	endif()

	message ( STATUS "Updating version information..." )
	# write version info to a temporary file
	configure_file ( "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/version.h~" )
	# update info iff changed
	execute_process ( COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/version.h~" "${CMAKE_CURRENT_SOURCE_DIR}/version.h" )
	# make sure info doesn't get stale
	file ( REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/version.h~" )
else()
	# -> tarball
	if ( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version.h" )
		message ( SEND_ERROR "The generated file 'version.h' does not exist!" )
		message ( AUTHOR_WARNING "When creating a release tarball, please make sure to run cmake -P ${CMAKE_CURRENT_LIST_FILE}" )
	endif()
endif()
