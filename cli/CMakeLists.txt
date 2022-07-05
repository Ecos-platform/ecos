file(DOWNLOAD
        https://raw.githubusercontent.com/badaix/popl/v1.3.0/include/popl.hpp
        ${CMAKE_BINARY_DIR}/thirdparty/popl/popl.hpp
        )
add_library(popl INTERFACE)
target_include_directories(popl INTERFACE ${CMAKE_BINARY_DIR}/thirdparty)

add_executable(main main.cpp)
target_link_libraries(main PUBLIC popl)
