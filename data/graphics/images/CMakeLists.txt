ADD_SUBDIRECTORY(ambient)

SET(FILES
    login_wallpaper.png
    minimap_new_1-1.png
    minimap_new_14-1.png
    minimap_new_15-1.png
    minimap_new_16-1.png
    minimap_new_2-1.png
    minimap_new_3-1.png
    minimap_new_5-1.png
    minimap_new_7-1.png
    minimap_new_9-1.png
    )

INSTALL(FILES ${FILES} DESTINATION ${DATA_DIR}/graphics/images)
