SET(FILES
    about.txt
    changes.txt
    commands.txt
    header.txt
    index.txt
    support.txt
    windows.txt
    )

INSTALL(FILES ${FILES} DESTINATION ${DATA_DIR}/help)
