macro(setup_iwyu_globally)
  find_program(IWYU_EXE 
    NAMES include-what-you-use iwyu
  )

  if (NOT IWYU_EXE)
    message(ERROR "IWYU not found")
  endif()

  message(STATUS "include-what-you-use found: ${IWYU_EXE}")
  set(IWYU_EXE "${IWYU_EXE}" "-Xiwyu" "--verbose=1"
                              "-Xiwyu" "--cxx17ns"
  )
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
  set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
  
endmacro(setup_iwyu_globally)