SET(FILES
    tmw-icon.ico
    tmw-icon.png
    tmw-icon.xpm
    )

INSTALL(FILES ${FILES} DESTINATION ${DATA_DIR}/icons)

INSTALL(FILES "tmw-icon.png" DESTINATION share/pixmaps RENAME tmw.png)
