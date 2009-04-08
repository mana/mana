FIND_PACKAGE(Gettext REQUIRED)

FILE(GLOB POFILES *.po)
GETTEXT_CREATE_TRANSLATIONS(tmw.pot ${POFILES})
# This is an ugly hack to enable building of translations with
# the default target due to a bug in GETTEXT_CREATE_TRANSLATIONS.
# The bug has been fixed in recent CMake 2.6.3. Once 2.6.3 is widely
# deployed the above line should be changed to:
# GETTEXT_CREATE_TRANSLATIONS(tmw.pot ALL ${POFILES}) 
# and the following two lines removed 
#     2009-04-08 Bernd Wachter <bwachter-tmw@lart.info>
ADD_CUSTOM_TARGET(transfix ALL)
ADD_DEPENDENCIES(transfix translations)
