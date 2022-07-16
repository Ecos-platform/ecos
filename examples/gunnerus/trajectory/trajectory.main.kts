@file:DependsOn("info.laht.sspgen:dsl:0.5.1")

import no.ntnu.ihb.sspgen.dsl.ssp
import kotlin.math.PI

ssp("gunnerus-trajectory") {

    println("Building SSP '$archiveName'..")

    resources {
        url("https://github.com/gunnerus-case/gunnerus-fmus-bin/raw/master/VesselFmu2.fmu")
        url("https://github.com/gunnerus-case/gunnerus-fmus-bin/raw/master/PMAzimuth.fmu")
        url("https://github.com/gunnerus-case/gunnerus-fmus-bin/raw/master/PowerPlant.fmu")
        url("https://github.com/gunnerus-case/gunnerus-fmus-bin/raw/master/ThrusterDrive2.fmu")
        url("https://github.com/gunnerus-case/gunnerus-fmus-bin/raw/master/TrajectoryController.fmu")
        url("https://github.com/gunnerus-case/gunnerus-fmus-bin/raw/master/WaypointProvider2DOF.fmu")
    }

    ssd("KPN Twinship Gunnerus case") {

        system("gunnerus-trajectory") {

            elements {

                component("vesselModel", "resources/VesselFmu2.fmu") {
                    connectors {
                        real("additionalBodyForce[0].force.heave", input)
                        real("additionalBodyForce[0].force.surge", input)
                        real("additionalBodyForce[0].force.sway", input)
                        real("additionalBodyForce[0].pointOfAttackRel2APAndBL.xpos", input)
                        real("additionalBodyForce[0].pointOfAttackRel2APAndBL.ypos", input)
                        real("additionalBodyForce[0].pointOfAttackRel2APAndBL.zpos", input)

                        real("additionalBodyForce[1].force.heave", input)
                        real("additionalBodyForce[1].force.surge", input)
                        real("additionalBodyForce[1].force.sway", input)
                        real("additionalBodyForce[1].pointOfAttackRel2APAndBL.xpos", input)
                        real("additionalBodyForce[1].pointOfAttackRel2APAndBL.ypos", input)
                        real("additionalBodyForce[1].pointOfAttackRel2APAndBL.zpos", input)

                        real("cg_x_rel_ap", output)
                        real("cg_y_rel_cl", output)
                        real("cg_z_rel_bl", output)
                        real("cgShipMotion.nedDisplacement.north", output)
                        real("cgShipMotion.nedDisplacement.east", output)
                        real("cgShipMotion.linearVelocity.surge", output)
                        real("cgShipMotion.linearVelocity.sway", output)
                        real("cgShipMotion.angularVelocity.yaw", output)
                        real("cgShipMotion.angularDisplacement.yaw", output)
                    }
                    parameterBindings {
                        parameterSet("initialValues") {
                            string("vesselZipFile", "%fmu%/resources/ShipModel-gunnerus-elongated.zip")
                            boolean("additionalBodyForce[0].enabled", true)
                            boolean("additionalBodyForce[1].enabled", true)
                        }
                    }
                }

                component("azimuth0", "proxyfmu://localhost?file=resources/PMAzimuth.fmu") {
                    connectors {
                        real("input_act_revs", input)
                        real("input_act_angle", input)
                        real("input_cg_x_rel_ap", input)
                        real("input_cg_y_rel_cl", input)
                        real("input_cg_z_rel_bl", input)
                        real("input_cg_surge_vel", input)
                        real("input_cg_sway_vel", input)
                        real("input_yaw_vel", input)

                        real("output_torque", output)
                        real("output_force_heave", output)
                        real("output_force_surge", output)
                        real("output_force_sway", output)
                        real("output_x_rel_ap", output)
                        real("output_y_rel_cl", output)
                        real("output_z_rel_bl", output)
                    }
                    parameterBindings {
                        parameterSet("initialValues") {
                            real("input_x_rel_ap", 1.5)
                            real("input_y_rel_cl", -2.7)
                            real("input_z_rel_bl", 0)
                            real("input_prop_diam", 1.9)
                            real("input_distancetohull", 1.5)
                            real("input_bilgeradius", 3)
                            real("input_rho", 1025)
                            real("input_lpp", 33.9)
                        }
                    }
                }

                component("azimuth1", "proxyfmu://localhost?file=resources/PMAzimuth.fmu") {
                    connectors {
                        copyFrom("azimuth0")
                    }
                    parameterBindings {
                        copyFrom("azimuth0", "initialValues") {
                            real("input_y_rel_cl", 2.7)
                        }
                    }
                }

                component("azimuth0_rpmActuator", "resources/ThrusterDrive2.fmu") {
                    connectors {
                        real("d_in.e", input)
                        real("q_in.e", input)
                        real("ThrustCom", input)
                        real("Shaft.e", input)

                        real("d_in.f", output)
                        real("q_in.f", output)
                        real("Shaft.f", output)
                    }
                }

                component("azimuth1_rpmActuator", "resources/ThrusterDrive2.fmu") {
                    connectors {
                        copyFrom("azimuth0_rpmActuator")
                    }
                }

                component("powerPlant", "resources/PowerPlant.fmu") {
                    connectors {
                        real("p1.f[1]", input)
                        real("p1.f[2]", input)
                        real("p2.f[1]", input)
                        real("p2.f[2]", input)

                        real("p1.e[1]", output)
                        real("p1.e[2]", output)
                        real("p2.e[1]", output)
                        real("p2.e[2]", output)
                    }
                }

                component("trackController", "resources/TrajectoryController.fmu") {
                    connectors {
                        real("northPosition", input)
                        real("eastPosition", input)
                        real("surgeVelocity", input)
                        real("swayVelocity", input)
                        real("headingAngle", input)
                        real("targetWP.north", input)
                        real("targetWP.east", input)
                        real("targetWP.speed", input)
                        real("prevWP.north", input)
                        real("prevWP.east", input)
                        real("prevWP.speed", input)

                        real("forceCommand", output)
                        real("rudderCommand", output)
                    }
                    parameterBindings {
                        parameterSet("initialValues") {
                            real("autopilot.heading.kp", 0.5)
                            real("autopilot.heading.ki", 0)
                            real("autopilot.heading.kd", 2)
                            real("autopilot.speed.kp", 1000)
                            real("autopilot.speed.ki", 100)
                            real("autopilot.speed.kd", 500)
                            real("lookaheadDistance", 50)
                            boolean("enable", false)
                            boolean("shouldLog", false)
                        }
                    }
                }

                component("wpProvider", "resources/WaypointProvider2DOF.fmu") {
                    connectors {
                        real("northPosition", input)
                        real("eastPosition", input)
                        real("headingAngle", input)

                        real("targetWP.north", output)
                        real("targetWP.east", output)
                        real("targetWP.speed", output)
                        real("prevWP.north", output)
                        real("prevWP.east", output)
                        real("prevWP.speed", output)
                    }
                    parameterBindings {
                        parameterSet("initialValues") {
                            real("wpSwitchDistance", 20)
                            boolean("shouldLog", false)
                        }
                    }
                }

            }

            connections(inputsFirst = true) {
                "trackController.northPosition" to "vesselModel.cgShipMotion.nedDisplacement.north"
                "trackController.eastPosition" to "vesselModel.cgShipMotion.nedDisplacement.east"
                "trackController.surgeVelocity" to "vesselModel.cgShipMotion.linearVelocity.surge"
                "trackController.swayVelocity" to "vesselModel.cgShipMotion.linearVelocity.sway"
                "trackController.headingAngle" to "vesselModel.cgShipMotion.angularDisplacement.yaw"
                "trackController.targetWP.north" to "wpProvider.targetWP.north"
                "trackController.targetWP.east" to "wpProvider.targetWP.east"
                "trackController.targetWP.speed" to "wpProvider.targetWP.speed"
                "trackController.prevWP.north" to "wpProvider.prevWP.north"
                "trackController.prevWP.east" to "wpProvider.prevWP.east"
                "trackController.prevWP.speed" to "wpProvider.prevWP.speed"

                "wpProvider.northPosition" to "vesselModel.cgShipMotion.nedDisplacement.north"
                "wpProvider.eastPosition" to "vesselModel.cgShipMotion.nedDisplacement.east"
                "wpProvider.headingAngle" to "vesselModel.cgShipMotion.angularDisplacement.yaw"

                "powerPlant.p1.f[1]" to "azimuth0_rpmActuator.d_in.f"
                "powerPlant.p1.f[2]" to "azimuth0_rpmActuator.q_in.f"
                "powerPlant.p2.f[1]" to "azimuth1_rpmActuator.d_in.f"
                "powerPlant.p2.f[2]" to "azimuth1_rpmActuator.q_in.f"

                "azimuth0_rpmActuator.d_in.e" to "powerPlant.p1.e[1]"
                "azimuth0_rpmActuator.q_in.e" to "powerPlant.p1.e[2]"
                "azimuth0_rpmActuator.ThrustCom" to "trackController.forceCommand"
                "azimuth0_rpmActuator.Shaft.e" to "azimuth0.output_torque"

                "azimuth1_rpmActuator.d_in.e" to "powerPlant.p2.e[1]"
                "azimuth1_rpmActuator.q_in.e" to "powerPlant.p2.e[2]"
                "azimuth1_rpmActuator.ThrustCom" to "trackController.forceCommand"
                "azimuth1_rpmActuator.Shaft.e" to "azimuth1.output_torque"

                ("azimuth0.input_act_revs" to "azimuth0_rpmActuator.Shaft.f").linearTransformation(factor = 60.0 / (2 * PI))
                "azimuth0.input_act_angle" to "trackController.rudderCommand"
                "azimuth0.input_cg_x_rel_ap" to "vesselModel.cg_x_rel_ap"
                "azimuth0.input_cg_y_rel_cl" to "vesselModel.cg_y_rel_cl"
                "azimuth0.input_cg_z_rel_bl" to "vesselModel.cg_z_rel_bl"
                "azimuth0.input_cg_surge_vel" to "vesselModel.cgShipMotion.linearVelocity.surge"
                "azimuth0.input_cg_sway_vel" to "vesselModel.cgShipMotion.linearVelocity.sway"
                "azimuth0.input_yaw_vel" to "vesselModel.cgShipMotion.angularVelocity.yaw"

                ("azimuth1.input_act_revs" to "azimuth1_rpmActuator.Shaft.f").linearTransformation(factor = 60.0 / (2 * PI))
                "azimuth1.input_act_angle" to "trackController.rudderCommand"
                "azimuth1.input_cg_x_rel_ap" to "vesselModel.cg_x_rel_ap"
                "azimuth1.input_cg_y_rel_cl" to "vesselModel.cg_y_rel_cl"
                "azimuth1.input_cg_z_rel_bl" to "vesselModel.cg_z_rel_bl"
                "azimuth1.input_cg_surge_vel" to "vesselModel.cgShipMotion.linearVelocity.surge"
                "azimuth1.input_cg_sway_vel" to "vesselModel.cgShipMotion.linearVelocity.sway"
                "azimuth1.input_yaw_vel" to "vesselModel.cgShipMotion.angularVelocity.yaw"

                "vesselModel.additionalBodyForce[0].force.heave" to "azimuth0.output_force_heave"
                "vesselModel.additionalBodyForce[0].force.surge" to "azimuth0.output_force_surge"
                "vesselModel.additionalBodyForce[0].force.sway" to "azimuth0.output_force_sway"
                "vesselModel.additionalBodyForce[0].pointOfAttackRel2APAndBL.xpos" to "azimuth0.output_x_rel_ap"
                "vesselModel.additionalBodyForce[0].pointOfAttackRel2APAndBL.ypos" to "azimuth0.output_y_rel_cl"
                "vesselModel.additionalBodyForce[0].pointOfAttackRel2APAndBL.zpos" to "azimuth0.output_z_rel_bl"

                "vesselModel.additionalBodyForce[1].force.heave" to "azimuth1.output_force_heave"
                "vesselModel.additionalBodyForce[1].force.surge" to "azimuth1.output_force_surge"
                "vesselModel.additionalBodyForce[1].force.sway" to "azimuth1.output_force_sway"
                "vesselModel.additionalBodyForce[1].pointOfAttackRel2APAndBL.xpos" to "azimuth1.output_x_rel_ap"
                "vesselModel.additionalBodyForce[1].pointOfAttackRel2APAndBL.ypos" to "azimuth1.output_y_rel_cl"
                "vesselModel.additionalBodyForce[1].pointOfAttackRel2APAndBL.zpos" to "azimuth1.output_z_rel_bl"
            }

        }

        defaultExperiment {
            annotations {
                annotation("com.opensimulationplatform") {
                    """
                        <osp:Algorithm>
                            <osp:FixedStepAlgorithm baseStepSize="0.05" />
                        </osp:Algorithm>
                    """
                }
            }
        }

        namespaces {
            namespace("osp", "http://opensimulationplatform.com/SSP/OSPAnnotations")
        }

    }


}.build().also { println("Done") }
