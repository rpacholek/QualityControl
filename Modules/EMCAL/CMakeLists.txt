# ---- Library ----

add_library(QcEMCAL src/DigitsQcTask.cxx)

target_include_directories(
  QcEMCAL
  PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(QcEMCAL PUBLIC QualityControl O2::EMCALBase)

add_root_dictionary(QcEMCAL
                    HEADERS include/EMCAL/DigitsQcTask.h
                    LINKDEF include/EMCAL/LinkDef.h
                    BASENAME QcEMCAL)

install(TARGETS QcEMCAL
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install (
  DIRECTORY etc DESTINATION Modules/EMCAL
)

# ---- Executables ----

add_executable(qcRunEmcalDigit src/runQCEMCALDigits.cxx)
target_link_libraries(qcRunEmcalDigit PUBLIC QualityControl QcEMCAL O2::DataFormatsEMCAL O2::EMCALBase O2::EMCALWorkflow)
install(
  TARGETS qcRunEmcalDigit
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Tests ----

set(
  TEST_SRCS
)

foreach(test ${TEST_SRCS})
  get_filename_component(test_name ${test} NAME)
  string(REGEX REPLACE ".cxx" "" test_name ${test_name})

  add_executable(${test_name} ${test})
  target_link_libraries(${test_name} PRIVATE ${MODULE_NAME} Boost::unit_test_framework)
  add_test(NAME ${test_name} COMMAND ${test_name})
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 60)
endforeach()
