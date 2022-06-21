cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(dialog)

set(SRCS
    dialog.c
    ./Container/container.c
    ./Controls/ctrlControl.c
    ./Controls/ctrlLabel.c
    ./Controls/ctrlLineEdit.c
)

set(dialog
    dialog.h
    ./Container/container.h
    ./Controls/ctrlControl.h
    ./Controls/ctrlLabel.h
    ./Controls/ctrlLineEdit.h
)

add_library(dialog ${SRCS} ${HEADERS})

# list(APPEND CMAKE_MODULE_PATH "/usr/lib/x86_64-linux-gnu")

# set(CMAKE_MODULE_PATH /usr/lib/x86_64-linux-gnu)

target_link_libraries(dialog
    SDL2
    SDL2_ttf
)
