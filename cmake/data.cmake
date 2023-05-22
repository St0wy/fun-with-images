file(GLOB_RECURSE DATA_FILES
    "data/*.json"
    "data/*.png"
    "data/*.jpg"
    "data/*.bmp"
    "data/*.hdr"
    "data/*.obj"
    "data/*.mtl"
    )

# Copy every data filde
foreach(DATA ${DATA_FILES})
    get_filename_component(FILE_NAME ${DATA} NAME)
    get_filename_component(PATH_NAME ${DATA} DIRECTORY)
    get_filename_component(EXTENSION ${DATA} EXT)
    file(RELATIVE_PATH PATH_NAME "${CMAKE_CURRENT_SOURCE_DIR}" ${PATH_NAME})
    set(DATA_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
    add_custom_command(
        OUTPUT ${DATA_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy
        ${DATA}
        ${DATA_OUTPUT}
        DEPENDS ${DATA})
    list(APPEND Data_OUTPUT_FILES ${DATA_OUTPUT})
endforeach(DATA)


add_custom_target(
    data_target
    DEPENDS ${Data_OUTPUT_FILES}
)
