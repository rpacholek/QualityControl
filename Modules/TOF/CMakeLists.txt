# ---- Library ----

add_library(QcTOF src/TOFTask.cxx src/TOFCheckRawsMulti.cxx
                  src/TOFCheckRawsTime.cxx src/TOFCheckRawsToT.cxx)

target_sources(QcTOF PRIVATE)

target_include_directories(
  QcTOF
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(QcTOF PUBLIC QualityControl)

install(TARGETS QcTOF
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# ---- ROOT dictionary ----

add_root_dictionary(QcTOF
                    HEADERS include/TOF/TOFTask.h
                            include/TOF/TOFCheckRawsMulti.h
                            include/TOF/TOFCheckRawsTime.h
                            include/TOF/TOFCheckRawsToT.h
                    LINKDEF include/TOF/LinkDef.h
                    BASENAME QcTOF)

# ---- Test(s) ----

add_executable(testQcTOF test/testTOF.cxx)
target_link_libraries(testQcTOF PRIVATE QcTOF Boost::unit_test_framework)
add_test(NAME testQcTOF COMMAND testQcTOF)
set_tests_properties(testQcTOF PROPERTIES TIMEOUT 60)

# ---- Executables ----

set(EXE_SRCS src/runTOF.cxx)

set(EXE_NAMES o2-qc-run-tof)

list(LENGTH EXE_SRCS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
  list(GET EXE_SRCS ${i} src)
  list(GET EXE_NAMES ${i} name)
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE QualityControl CURL::libcurl)
endforeach()

# ---- Extra scripts ----

install(FILES tof.json DESTINATION etc)

get_property(dirs
             DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
