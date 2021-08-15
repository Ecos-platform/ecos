
include_directories(${VICO_SOURCE_DIR}/include)
include_directories(${VICO_SOURCE_DIR}/src)

add_executable(fmi-test fmi_test.cpp)
target_link_libraries(fmi-test PUBLIC fmilibwrapper)
