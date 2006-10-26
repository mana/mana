SET(FILES
    about.txt
    changes.txt
    commands.txt
    header.txt
    index.txt
    skills.txt
    support.txt
    team.txt
    )

INSTALL(FILES ${FILES} DESTINATION ${DATA_DIR}/help)
