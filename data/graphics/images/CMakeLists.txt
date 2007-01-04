ADD_SUBDIRECTORY(ambient)

SET(FILES
    error.png
    login_wallpaper.png
    )

INSTALL(FILES ${FILES} DESTINATION ${DATA_DIR}/graphics/images)
