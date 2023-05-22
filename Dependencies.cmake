include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(fun_with_images_setup_dependencies)

    # For each dependency, see if it's
    # already been provided to us by a parent project

    if(NOT TARGET fmtlib::fmtlib)
        cpmaddpackage("gh:fmtlib/fmt#9.1.0")
    endif()

    if(NOT TARGET spdlog::spdlog)
        cpmaddpackage(
            NAME
            spdlog
            VERSION
            1.11.0
            GITHUB_REPOSITORY
            "gabime/spdlog"
            OPTIONS
            "SPDLOG_FMT_EXTERNAL ON")
    endif()

    if(NOT TARGET Catch2::Catch2WithMain)
        cpmaddpackage("gh:catchorg/Catch2@3.3.2")
    endif()

    if(NOT TARGET minifb)
        cpmaddpackage("gh:emoon/minifb#master")
    endif()

    add_library(stb_image "external/stb_image/stb_image.h")
    set_target_properties(stb_image PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(stb_image PUBLIC external/stb_image/)

endfunction()
