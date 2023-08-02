find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
)

if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  file(SHA1 ${CMAKE_SOURCE_DIR}/.clang-tidy clang_tidy_sha1)
  set(CLANG_TIDY_DEFINITIONS "CLANG_TIDY_SHA1=${clang_tidy_sha1}")
  unset(clang_tidy_sha1)

  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-p=${CMAKE_BINARY_DIR}" "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
endif()

configure_file(${CMAKE_SOURCE_DIR}/.clang-tidy ${CMAKE_SOURCE_DIR}/.clang-tidy COPYONLY)  

macro(setup_clang_tidy TARGET SCOPE)
  set_target_properties(${TARGET} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
  target_compile_definitions(${TARGET} ${SCOPE} ${CLANG_TIDY_DEFINITIONS})
endmacro(setup_clang_tidy)