link_libraries(libecos CONAN_PKG::spdlog)
add_compile_definitions(DATA_FOLDER="${CMAKE_SOURCE_DIR}/data")

add_executable(controlled_temperature controlled_temperature/controlled_temperature.cpp)
add_executable(controlled_temperature_proxy controlled_temperature/controlled_temperature_proxy.cpp)

if (WIN32)
    add_executable(quarter_truck quarter_truck/quarter_truck.cpp)
    add_executable(quarter_truck_ssp quarter_truck/quarter_truck_ssp.cpp)

    add_executable(gunnerus_trajectory gunnerus/gunnerus_trajectory.cpp)
endif ()