link_libraries(libecos CONAN_PKG::spdlog)
add_compile_definitions(DATA_FOLDER="${CMAKE_SOURCE_DIR}/data")

add_executable(controlled_temperature controlled_temperature/controlled_temperature.cpp)
add_executable(controlled_temperature_proxy controlled_temperature/controlled_temperature_proxy.cpp)

if (WIN32)
    add_executable(quarter_truck quarter_truck/quarter_truck.cpp)
    add_executable(quarter_truck_ssp quarter_truck/quarter_truck_ssp.cpp)

    add_executable(gunnerus_trajectory gunnerus/trajectory/gunnerus_trajectory.cpp)
    target_compile_definitions(gunnerus_trajectory PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/gunnerus/trajectory")
endif ()
