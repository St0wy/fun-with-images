add_executable(fun-images main.cpp)

target_link_libraries(
    fun-images
    PRIVATE 
    fun_with_images::fun_with_images_options
    fun_with_images::fun_with_images_warnings
)

target_link_system_libraries(
    fun-images
    PRIVATE
        fmt::fmt
        spdlog::spdlog
        stb_image
        minifb
        )
add_dependencies(fun-images data_target)
target_include_directories(fun-images PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")