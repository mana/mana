find_package(Gettext REQUIRED)

# based on the CMake 2.6.3 GETTEXT_CREATE_TRANSLATIONS we need our own version
# because the upstream one contains a call to msgmerge (updating po files) which
# can't be disabled
macro(MANA_GETTEXT_CREATE_TRANSLATIONS _potFile _firstPoFileArg)
  # make it a real variable, so we can modify it here
  set(_firstPoFile "${_firstPoFileArg}")

  set(_gmoFiles)
  get_filename_component(_potBasename ${_potFile} NAME_WE)
  get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

  set(_addToAll)
  if(${_firstPoFile} STREQUAL "ALL")
    set(_addToAll "ALL")
    set(_firstPoFile)
  endif(${_firstPoFile} STREQUAL "ALL")

  foreach(_currentPoFile ${_firstPoFile} ${ARGN})
    get_filename_component(_absFile ${_currentPoFile} ABSOLUTE)
    get_filename_component(_abs_PATH ${_absFile} PATH)
    get_filename_component(_lang ${_absFile} NAME_WE)
    set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

    add_custom_command(
      OUTPUT ${_gmoFile}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
      DEPENDS ${_absPotFile} ${_absFile})

    install(
      FILES ${_gmoFile}
      DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/${_lang}/LC_MESSAGES
      RENAME ${_potBasename}.mo)
    set(_gmoFiles ${_gmoFiles} ${_gmoFile})

  endforeach(_currentPoFile)

  add_custom_target(
    translations
    ${_addToAll}
    DEPENDS ${_gmoFiles})
endmacro()

macro(MANA_GETTEXT_UPDATE_PO _potFile _languages)
  get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

  add_custom_target(
    update-pot COMMAND xgettext --files-from=translatable-files --directory=.
                       --output=${_potFile} -d mana --keyword=_ --keyword=N_)

  add_custom_target(update-translatable-files COMMAND grep '_\(' -Irl ../src |
                                                      sort > translatable-files)

  foreach(_lang ${_languages})
    get_filename_component(_absFile "${_lang}.po" ABSOLUTE)
    set(_currentPoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.po)
    add_custom_target(
      "update-translation-${_lang}"
      COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s
              ${_absFile} ${_absPotFile})
    add_dependencies("update-translation-${_lang}" update-pot)
    add_dependencies(update-po "update-translation-${_lang}")
    add_dependencies(update-pot update-translatable-files)
  endforeach()
endmacro()

set(languages)
set(POFILES)
file(STRINGS LINGUAS _languages)
foreach(_lang ${_languages})
  string(REGEX REPLACE "#.*" "" _lang "${_lang}")
  if(NOT ${_lang} STREQUAL "")
    set(languages ${languages} ${_lang})
    set(POFILES ${POFILES} "${_lang}.po")
  endif()
endforeach()

add_custom_target(update-po)

mana_gettext_create_translations(mana.pot ALL ${POFILES})
mana_gettext_update_po(mana.pot "${languages}")
