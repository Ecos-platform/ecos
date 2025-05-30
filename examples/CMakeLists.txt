
function(add_example_executable name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE libecos)
    target_compile_definitions(${name} PRIVATE DATA_FOLDER="${PROJECT_SOURCE_DIR}/data")
endfunction()


add_example_executable(dp_ship dp-ship/dp_ship.cpp)
add_example_executable(van_der_pol vanderpol/van_der_pol.cpp)
add_example_executable(bouncing_ball bouncing_ball/bouncing_ball.cpp)
add_example_executable(spring_mass_damper spring_mass_damper/spring_mass_damper.cpp)
add_example_executable(controlled_temperature controlled_temperature/controlled_temperature.cpp)

if (ECOS_WITH_PROXYFMU)
    add_example_executable(bouncing_ball_proxy bouncing_ball/bouncing_ball_proxy.cpp)
    add_example_executable(spring_mass_damper_proxy spring_mass_damper/spring_mass_damper_proxy.cpp)
    add_example_executable(controlled_temperature_proxy controlled_temperature/controlled_temperature_proxy.cpp)
endif ()

if (WIN32)
    add_example_executable(quarter_truck quarter_truck/quarter_truck.cpp)
    add_example_executable(quarter_truck_c quarter_truck/quarter_truck_c.cpp)
    target_link_libraries(quarter_truck_c PRIVATE libecosc)
    add_example_executable(quarter_truck_ssp quarter_truck/quarter_truck_ssp.cpp)
endif ()
