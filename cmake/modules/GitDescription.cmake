# - GIT_GET_DESCRIPTION ( description_variable [GIT_ARGS arg1 ...] [SEND_ERROR] )
# This macro calls git describe to obtain a concise description of the current
# git repository, i.e. the most recent tag, together with thea "distance" to
# that tag and the short hash of the current commit.
#
# The description will be written into 'description_variable'. If an error
# occurred, the variable will be set to '-NOTFOUND' unless SEND_ERROR is set.
# In this case the a SEND_ERROR will be issued and generation will be skipped.
#
# Additional arguments to "git described" can be specified after the keyword
# GIT_ARGS.
#
# Example invocation:
#  GIT_GET_DESCRIPTION ( PROJECT_VERSION GIT_ARGS --dirty )
#  message ( STATUS "Project is at revision ${PROJECT_VERSION}." )
#
# Assuming the project has been previously tagged with "1.0", there have been 2
# commits since, and the working copy has been changed, this example could print
# the following text:
#  -- Project is at revision 1.0-2-g58a35d9-dirty.
#
# Depends on CMakeParseArguments.
##

# SPDX-FileCopyrightText: 2012-2013 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

# FindGit is a standard module since cmake 2.8.2
# CMakeParseArguments is included since cmake 2.8.3
include ( CMakeParseArguments )

## git_get_description ( <DESCRIPTION_VARIABLE> [GIT_ARGS <arg1>..<argN>] [SEND_ERROR] )
# Write the result of "git describe" into the variable DESCRIPTION_VARIABLE.
# Additional arguments to git describe ( e.g. --dirty ) can be passed using the keyword GIT_ARGS.
# If SEND_ERROR is set, execution is immediately stopped when an error occurs.
function ( git_get_description DESCVAR )
	cmake_parse_arguments ( _GGD "SEND_ERROR" "" "GIT_ARGS" "${ARGN}" )
	if ( SEND_ERROR )
		set ( _severity SEND_ERROR )
	else()
		set ( _severity WARNING )
	endif()

	find_package ( Git QUIET )
	if ( NOT GIT_FOUND )
		message ( ${severity} "git_get_description: could not find package git!" )
		set ( ${DESCVAR} "-NOTFOUND" PARENT_SCOPE )
		return()
	endif()

	execute_process ( COMMAND "${GIT_EXECUTABLE}" describe ${_GGD_GIT_ARGS}
		WORKING_DIRECTORY "${BASE_DIR}"
		RESULT_VARIABLE _gitresult
		OUTPUT_VARIABLE _gitdesc
		ERROR_VARIABLE  _giterror
		OUTPUT_STRIP_TRAILING_WHITESPACE )

	if ( NOT _gitresult EQUAL 0 )
		message ( ${_severity} "git_get_description: error during execution of git describe!" )
		message ( ${_severity} "Error was: ${_giterror}" )
		set ( ${DESCVAR} "-NOTFOUND" PARENT_SCOPE )
	else()
		set ( ${DESCVAR} "${_gitdesc}" PARENT_SCOPE )
	endif()
endfunction()
