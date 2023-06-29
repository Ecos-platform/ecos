#include <catch2/catch_test_macros.hpp>

#include "ecos/fmi/proxy/proxy_fmu.hpp"

#include <unordered_map>

using namespace ecos::fmi;
using namespace ecos::proxyfmu;

namespace
{

const double tEnd = 5;
const double dt = 0.01;


unsigned int getRealValueRef(const model_variables& mv, const std::string& name)
{
    return std::find_if(mv.begin(), mv.end(), [name](const scalar_variable& v) {
        return v.name == name;
    })->vr;
}

void test(fmu* damper, fmu* mass, fmu* spring)
{

    std::vector<std::string> names{"damper", "mass", "spring"};
    std::unordered_map<std::string, std::unique_ptr<slave>> slaves;

    int i = 0;
    for (auto fmu : {damper, mass, spring}) {
        const auto name = names[i++];
        auto instance = fmu->new_instance(name);
        const auto& mv = fmu->get_model_description().modelVariables;

        if (name == "spring") {
            std::vector<unsigned int> vr = {
                getRealValueRef(mv, "springStiffness"),
                getRealValueRef(mv, "zeroForceLength")};
            std::vector<double> values = {5, 5};
            instance->set_real(vr, values);
        } else if (name == "damper") {
            std::vector<unsigned int> vr = {
                getRealValueRef(mv, "dampingCoefficient")};
            std::vector<double> values = {2};
            instance->set_real(vr, values);
        } else if (name == "mass") {
            std::vector<unsigned int> vr = {
                getRealValueRef(mv, "initialPositionX"),
                getRealValueRef(mv, "mediumDensity")};
            std::vector<double> values = {5, 1};
            instance->set_real(vr, values);
        }
        slaves[name] = std::move(instance);
    }

    unsigned int spring_for_xx = getRealValueRef(slaves["spring"]->get_model_description().modelVariables, "for_xx");
    unsigned int spring_for_yx = getRealValueRef(slaves["spring"]->get_model_description().modelVariables, "for_yx");
    unsigned int spring_dis_xx = getRealValueRef(slaves["spring"]->get_model_description().modelVariables, "dis_xx");
    unsigned int spring_dis_yx = getRealValueRef(slaves["spring"]->get_model_description().modelVariables, "dis_yx");

    unsigned int mass_in_f_u = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "in_f_u");
    unsigned int mass_in_f_w = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "in_f_w");
    unsigned int mass_out_f_u = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "out_f_u");
    unsigned int mass_out_f_w = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "out_f_w");
    unsigned int mass_in_l_u = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "in_l_u");
    unsigned int mass_in_l_w = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "in_l_w");
    unsigned int mass_out_l_u = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "out_l_u");
    unsigned int mass_out_l_w = getRealValueRef(slaves["mass"]->get_model_description().modelVariables, "out_l_w");

    unsigned int damper_df_0 = getRealValueRef(slaves["damper"]->get_model_description().modelVariables, "df_0");
    unsigned int damper_df_1 = getRealValueRef(slaves["damper"]->get_model_description().modelVariables, "df_1");
    unsigned int damper_lv_0 = getRealValueRef(slaves["damper"]->get_model_description().modelVariables, "lv_0");
    unsigned int damper_lv_1 = getRealValueRef(slaves["damper"]->get_model_description().modelVariables, "lv_1");

    std::vector<double> spring_out(2);
    std::vector<double> damper_out(2);
    std::vector<double> mass_spring_out(2);
    std::vector<double> mass_damper_out(2);

    std::vector<unsigned int> spring_in_vr = {spring_dis_xx, spring_dis_yx};
    std::vector<unsigned int> spring_out_vr = {spring_for_xx, spring_for_yx};

    std::vector<unsigned int> damper_in_vr = {damper_lv_0, damper_lv_1};
    std::vector<unsigned int> damper_out_vr = {damper_df_0, damper_df_1};

    std::vector<unsigned int> mass_spring_in_vr = {mass_in_l_u, mass_in_l_w};
    std::vector<unsigned int> mass_damper_in_vr = {mass_in_f_u, mass_in_f_w};
    std::vector<unsigned int> mass_spring_out_vr = {mass_out_l_u, mass_out_l_w};
    std::vector<unsigned int> mass_damper_out_vr = {mass_out_f_u, mass_out_f_w};

    for (auto& [name, instance] : slaves) {
        REQUIRE(instance->setup_experiment());
        REQUIRE(instance->enter_initialization_mode());
    }

    for (unsigned j = 0; j < slaves.size(); ++j) {
        slaves["spring"]->get_real(spring_out_vr, spring_out);
        slaves["damper"]->get_real(damper_out_vr, damper_out);
        slaves["mass"]->get_real(mass_spring_out_vr, mass_spring_out);
        slaves["mass"]->get_real(mass_damper_out_vr, mass_damper_out);

        slaves["mass"]->set_real(mass_spring_in_vr, spring_out);
        slaves["mass"]->set_real(mass_damper_in_vr, damper_out);

        slaves["spring"]->set_real(spring_in_vr, mass_spring_out);
        slaves["damper"]->set_real(damper_in_vr, mass_damper_out);
    }

    for (auto& [name, instance] : slaves) {
        REQUIRE(instance->exit_initialization_mode());
    }

    slaves["spring"]->get_real(spring_out_vr, spring_out);
    slaves["damper"]->get_real(damper_out_vr, damper_out);
    slaves["mass"]->get_real(mass_spring_out_vr, mass_spring_out);
    slaves["mass"]->get_real(mass_damper_out_vr, mass_damper_out);

    double t = 0.0;
    while (t <= tEnd) {

        slaves["mass"]->set_real(mass_spring_in_vr, spring_out);
        slaves["mass"]->set_real(mass_damper_in_vr, damper_out);

        slaves["spring"]->set_real(spring_in_vr, mass_spring_out);
        slaves["damper"]->set_real(damper_in_vr, mass_damper_out);

        for (auto& [name, slave] : slaves) {
            REQUIRE(slave->step(t, dt));
        }

        slaves["spring"]->get_real(spring_out_vr, spring_out);
        slaves["damper"]->get_real(damper_out_vr, damper_out);
        slaves["mass"]->get_real(mass_spring_out_vr, mass_spring_out);
        slaves["mass"]->get_real(mass_damper_out_vr, mass_damper_out);

        t += dt;
    }

    for (auto& [name, slave] : slaves) {
        REQUIRE(slave->terminate());
        slave->freeInstance();
    }
}

} // namespace

TEST_CASE("fmi_test_mass-spring-damper")
{
    auto damper = loadFmu(std::string(DATA_FOLDER) + "/fmus/1.0/Damper.fmu");
    REQUIRE(damper);
    auto mass = loadFmu(std::string(DATA_FOLDER) + "/fmus/1.0/Mass.fmu");
    REQUIRE(mass);
    auto spring = loadFmu(std::string(DATA_FOLDER) + "/fmus/1.0/Spring.fmu");
    REQUIRE(spring);
    test(damper.get(), mass.get(), spring.get());
}

TEST_CASE("client_test_mass-spring-damper")
{
    auto damper = proxy_fmu(std::string(DATA_FOLDER) + "/fmus/1.0/Damper.fmu");
    auto mass = proxy_fmu(std::string(DATA_FOLDER) + "/fmus/1.0/Mass.fmu");
    auto spring = proxy_fmu(std::string(DATA_FOLDER) + "/fmus/1.0/Spring.fmu");
    test(&damper, &mass, &spring);
}
