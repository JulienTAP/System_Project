set(APP_NAME ShellProgram)

set(CWD ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${APP_NAME})

set(src
    ${CWD}/main.c
)

target_sources(${APP_NAME} PRIVATE ${src})
target_include_directories(${APP_NAME} PRIVATE ${INCLUDES_DIR})