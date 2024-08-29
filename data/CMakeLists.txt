file(GLOB_RECURSE DATA_FILES
     fonts/*.ttf
     graphics/*.png
     graphics/*.xml
     help/*.txt
     icons/*.hqx
     icons/*.icns
     icons/*.ico
     icons/*.png
     icons/*.svg
     icons/*.xpm
     music/*.ogg
     sfx/*.ogg
     sfx/*.wav)

# Add a custom target to make sure the data files are visible in IDEs
add_custom_target(data SOURCES ${DATA_FILES})

install(DIRECTORY fonts graphics help icons music sfx
        DESTINATION ${CMAKE_INSTALL_DATADIR}/data)
