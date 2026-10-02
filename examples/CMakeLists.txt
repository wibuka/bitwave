add_executable(play_simple play_simple.c)
target_include_directories(play_simple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(play_simple PRIVATE bitwave)

add_custom_command(TARGET play_simple POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/DL3PDAlois02.mp3"
        "$<TARGET_FILE_DIR:play_simple>/DL3PDAlois02.mp3"
)

add_executable(play_notes play_notes.c)
target_include_directories(play_notes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(play_notes PRIVATE bitwave)
