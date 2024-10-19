
function(add_example_executable name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE libecos spdlog::spdlog)
    target_compile_definitions(${name} PRIVATE DATA_FOLDER="${PROJECT_SOURCE_DIR}/data")
endfunction()

add_example_executable(controlled_temperature controlled_temperature/controlled_temperature.cpp)
add_example_executable(controlled_temperature_proxy controlled_temperature/controlled_temperature_proxy.cpp)

add_example_executable(spring_mass_damper spring_mass_damper/spring_mass_damper.cpp)
add_example_executable(spring_mass_damper_proxy spring_mass_damper/spring_mass_damper_proxy.cpp)

if (WIN32)
    add_example_executable(quarter_truck quarter_truck/quarter_truck.cpp)
    add_example_executable(quarter_truck_ssp quarter_truck/quarter_truck_ssp.cpp)

    add_example_executable(gunnerus_trajectory gunnerus/trajectory/gunnerus_trajectory.cpp)
    target_compile_definitions(gunnerus_trajectory PRIVATE SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/gunnerus/trajectory")
endif ()
