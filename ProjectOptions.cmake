include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(fun_with_images_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(fun_with_images_setup_options)
  option(fun_with_images_ENABLE_HARDENING "Enable hardening" ON)
  option(fun_with_images_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    fun_with_images_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    fun_with_images_ENABLE_HARDENING
    OFF)

  fun_with_images_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR fun_with_images_PACKAGING_MAINTAINER_MODE)
    option(fun_with_images_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(fun_with_images_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(fun_with_images_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(fun_with_images_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(fun_with_images_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(fun_with_images_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(fun_with_images_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(fun_with_images_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(fun_with_images_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(fun_with_images_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(fun_with_images_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(fun_with_images_ENABLE_PCH "Enable precompiled headers" OFF)
    option(fun_with_images_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(fun_with_images_ENABLE_IPO "Enable IPO/LTO" ON)
    option(fun_with_images_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(fun_with_images_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(fun_with_images_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(fun_with_images_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(fun_with_images_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(fun_with_images_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(fun_with_images_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(fun_with_images_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(fun_with_images_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(fun_with_images_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(fun_with_images_ENABLE_PCH "Enable precompiled headers" OFF)
    option(fun_with_images_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      fun_with_images_ENABLE_IPO
      fun_with_images_WARNINGS_AS_ERRORS
      fun_with_images_ENABLE_USER_LINKER
      fun_with_images_ENABLE_SANITIZER_ADDRESS
      fun_with_images_ENABLE_SANITIZER_LEAK
      fun_with_images_ENABLE_SANITIZER_UNDEFINED
      fun_with_images_ENABLE_SANITIZER_THREAD
      fun_with_images_ENABLE_SANITIZER_MEMORY
      fun_with_images_ENABLE_UNITY_BUILD
      fun_with_images_ENABLE_CLANG_TIDY
      fun_with_images_ENABLE_CPPCHECK
      fun_with_images_ENABLE_COVERAGE
      fun_with_images_ENABLE_PCH
      fun_with_images_ENABLE_CACHE)
  endif()

  fun_with_images_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (fun_with_images_ENABLE_SANITIZER_ADDRESS OR fun_with_images_ENABLE_SANITIZER_THREAD OR fun_with_images_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(fun_with_images_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(fun_with_images_global_options)
  if(fun_with_images_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    fun_with_images_enable_ipo()
  endif()

  fun_with_images_supports_sanitizers()

  if(fun_with_images_ENABLE_HARDENING AND fun_with_images_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR fun_with_images_ENABLE_SANITIZER_UNDEFINED
       OR fun_with_images_ENABLE_SANITIZER_ADDRESS
       OR fun_with_images_ENABLE_SANITIZER_THREAD
       OR fun_with_images_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${fun_with_images_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${fun_with_images_ENABLE_SANITIZER_UNDEFINED}")
    fun_with_images_enable_hardening(fun_with_images_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(fun_with_images_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(fun_with_images_warnings INTERFACE)
  add_library(fun_with_images_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  fun_with_images_set_project_warnings(
    fun_with_images_warnings
    ${fun_with_images_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(fun_with_images_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(fun_with_images_options)
  endif()

  include(cmake/Sanitizers.cmake)
  fun_with_images_enable_sanitizers(
    fun_with_images_options
    ${fun_with_images_ENABLE_SANITIZER_ADDRESS}
    ${fun_with_images_ENABLE_SANITIZER_LEAK}
    ${fun_with_images_ENABLE_SANITIZER_UNDEFINED}
    ${fun_with_images_ENABLE_SANITIZER_THREAD}
    ${fun_with_images_ENABLE_SANITIZER_MEMORY})

  set_target_properties(fun_with_images_options PROPERTIES UNITY_BUILD ${fun_with_images_ENABLE_UNITY_BUILD})

  if(fun_with_images_ENABLE_PCH)
    target_precompile_headers(
      fun_with_images_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(fun_with_images_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    fun_with_images_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(fun_with_images_ENABLE_CLANG_TIDY)
    fun_with_images_enable_clang_tidy(fun_with_images_options ${fun_with_images_WARNINGS_AS_ERRORS})
  endif()

  if(fun_with_images_ENABLE_CPPCHECK)
    fun_with_images_enable_cppcheck(${fun_with_images_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(fun_with_images_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    fun_with_images_enable_coverage(fun_with_images_options)
  endif()

  if(fun_with_images_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(fun_with_images_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(fun_with_images_ENABLE_HARDENING AND NOT fun_with_images_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR fun_with_images_ENABLE_SANITIZER_UNDEFINED
       OR fun_with_images_ENABLE_SANITIZER_ADDRESS
       OR fun_with_images_ENABLE_SANITIZER_THREAD
       OR fun_with_images_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    fun_with_images_enable_hardening(fun_with_images_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
