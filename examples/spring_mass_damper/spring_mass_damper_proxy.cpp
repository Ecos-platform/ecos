
#include "smd_common.hpp"

int main()
{

    log::set_logging_level(log::level::trace);

    simulation_structure ss;
    ss.add_model("mass", "proxyfmu://localhost?file=" + std::string(DATA_FOLDER) + "/fmus/1.0/Mass.fmu");
    ss.add_model("spring", "proxyfmu://localhost?file=" + std::string(DATA_FOLDER) + "/fmus/1.0/Spring.fmu");
    ss.add_model("damper", "proxyfmu://localhost?file=" + std::string(DATA_FOLDER) + "/fmus/1.0/Damper.fmu");

    ss.make_connection<double>("spring::for_xx", "mass::in_l_u");
    ss.make_connection<double>("spring::for_yx", "mass::in_l_w");
    ss.make_connection<double>("mass::out_l_u", "spring::dis_xx");
    ss.make_connection<double>("mass::out_l_w", "spring::dis_yx");
    ss.make_connection<double>("damper::df_0", "mass::in_f_u");
    ss.make_connection<double>("damper::df_1", "mass::in_f_w");
    ss.make_connection<double>("mass::out_f_u", "damper::lv_0");
    ss.make_connection<double>("mass::out_f_w", "damper::lv_1");

    std::map<variable_identifier, scalar_value> map;
    map["spring::springStiffness"] = 5.;
    map["spring::zeroForceLength"] = 5.;
    map["damper::dampingCoefficient"] = 2.;
    map["mass::initialPositionX"] = 6.;
    map["mass::mediumDensity"] = 1.;
    ss.add_parameter_set("initialValues", map);

    run(ss);

}