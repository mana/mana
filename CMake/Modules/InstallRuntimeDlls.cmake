# Installs the DLLs needed by MANA_EXECUTABLE, looking for them in
# MANA_SEARCH_DIRS. DLLs that are not found are assumed to be part of Windows.
#
# This exists because install(TARGETS ... RUNTIME_DEPENDENCIES) refuses to run
# when cross-compiling, while file(GET_RUNTIME_DEPENDENCIES) only finds DLLs
# whose file name happens to be lower case when the search directories are on a
# case-sensitive file system.

function(_mana_find_dll name out)
  string(TOLOWER "${name}" lower_name)
  foreach(dir IN LISTS MANA_SEARCH_DIRS)
    file(GLOB candidates "${dir}/*.dll" "${dir}/*.DLL")
    foreach(candidate IN LISTS candidates)
      get_filename_component(candidate_name "${candidate}" NAME)
      string(TOLOWER "${candidate_name}" lower_candidate_name)
      if(lower_candidate_name STREQUAL lower_name)
        set(${out} "${candidate}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
  endforeach()
  set(${out} "" PARENT_SCOPE)
endfunction()

set(pending "${MANA_EXECUTABLE}" ${MANA_EXTRA_DLLS})
set(resolved)
set(seen)

while(pending)
  list(POP_FRONT pending current)

  if(NOT IS_ABSOLUTE "${current}")
    _mana_find_dll("${current}" found)
    if(NOT found)
      # Not shipped with the application, so provided by Windows itself
      continue()
    endif()
    set(current "${found}")
    list(APPEND resolved "${current}")
  endif()

  execute_process(
    COMMAND "${MANA_OBJDUMP}" -p "${current}"
    OUTPUT_VARIABLE headers
    COMMAND_ERROR_IS_FATAL ANY)

  string(REGEX MATCHALL "DLL Name: [^\r\n]+" imports "${headers}")
  foreach(import IN LISTS imports)
    string(REGEX REPLACE "DLL Name: +" "" import "${import}")
    string(TOLOWER "${import}" lower_import)
    if(NOT lower_import IN_LIST seen)
      list(APPEND seen "${lower_import}")
      list(APPEND pending "${import}")
    endif()
  endforeach()
endwhile()

file(
  INSTALL
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${MANA_DESTINATION}"
  TYPE SHARED_LIBRARY
  FOLLOW_SYMLINK_CHAIN
  FILES ${resolved})
