@file:DependsOn("info.laht.sspgen:dsl:0.5.2")

import no.ntnu.ihb.sspgen.dsl.ssp

ssp("quarter_truck_sspgen") {

    resources {

        file("resources/chassis.fmu")
        file("resources/wheel.fmu")
        file("resources/ground.fmu")
    }

    ssd("QuarterTruck") {

        system("QuarterTruck") {

            elements {

                component("chassis", "resources/chassis.fmu") {
                    connectors {
                        real("p.e", output)
                        real("p.f", input)
                    }
                    parameterBindings {
                        parameterSet("initialValues") {
                            real("C.mChassis", 400)
                            real("C.kChassis", 15000)
                            real("R.dChassis", 1000)
                        }
                    }
                }

                component("wheel", "resources/wheel.fmu") {
                    connectors {
                        real("p.f", input)
                        real("p1.e", input)
                        real("p.e", output)
                        real("p1.f", output)
                    }
                    parameterBindings {
                        parameterSet("initialValues") {
                            real("C.mWheel", 40)
                            real("C.kWheel", 150000)
                            real("R.dWheel", 0)
                        }
                    }
                }

                component("ground", "resources/ground.fmu") {
                    connectors {
                        real("p.e", input)
                        real("p.f", output)
                    }
                }

            }

            ospConnections {
                "chassis.linear mechanical port" to "wheel.chassis port"
                "wheel.ground port" to "ground.linear mechanical port"
            }

        }

        defaultExperiment {
            annotations {
                annotation("com.opensimulationplatform") {
                    """
                        <osp:Algorithm>
                            <osp:FixedStepAlgorithm baseStepSize="0.001" />
                        </osp:Algorithm>
                    """
                }
            }
        }

        namespaces {
            namespace("osp", "http://opensimulationplatform.com/SSP/OSPAnnotations")
        }

    }

}.build()
