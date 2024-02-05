

aux_source_directory(. SERVER_LIST)



add_executable(Server ${SERVER_LIST})

target_link_libraries(Server)