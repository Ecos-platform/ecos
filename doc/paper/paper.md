---
title: 'Ecos: A simple and intuitive co-simulation framework'
tags:
  - Functional Mockup Interface
  - Simulation and Modelling
  - C/C++
  - Python
authors:
  - name: Lars Ivar Hatledal
    orcid: 0000-0001-6436-7213
    affiliation: 1

affiliations:
 - name: Norwegian University of Science and Technology, Department of ICT and Natural Sciences, Norway
   index: 1
date: 14 April 2025
bibliography: paper.bib
---

# Summary

Ecos is cross-platform framework for running co-simulations adhering to the Functional Mock-up Interface (FMI) standard [@blochwitz2011functional]; 
an open standard for model exchange and co-simulation of dynamic systems.
An FMU (Functional Mock-up Unit) is a self-contained component that implements the FMI standard.
It is packaged as a zip archive containing:
- A shared library for each supported platform, which implements a standardized C interface.
- A *modelDescription.xml* file, describing the FMU's capabilities and available variables.
- Optionally, component-specific resources embedded within the archive.

The intention of Ecos is to provide a streamlined way of working with such FMUs, and support version 1.0, 2.0 and 3.0 of the standard with respect to co-simulation.
Ecos consists of a Command Line Interface (CLI), as well as a C++ library, _libecos_, with interfaces provided in C and Python. The Python package is available on pypy as _ecospy_.
The project is structured as a mono-repo with a major goal of being simple to build. This also implies few and light-weight dependencies.

Some features available with Ecos:

* Support for SSP 1.0.
* Support for FMI 1.0, 2.0 & 3.0 for Co-simulation.
* Built-in plotting capabilities with inline and XML configuration options.
* CSV writer with inline and XML configuration options.
* Scenarios; actions to run at specific events.
* Remoting - allowing simulations to run across processes.


\autoref{fig:mass_spring_damper}(figures/mass_spring_damper.png) demonstrates a simulation of a simple *mass-spring-damper* system with Ecos. 
The models are packaged following the System Structure & Parameterization (SSP) standard[@kohler2016modelica]
and subsequently simulated and plotted with _libecos_.


# Statement of need

While similar tooling exists like FMPy[@FMPy], Vico[@hatledal2021vico], Open Simulation Platform [@smogeli2020open] and OMSimulator[@ochel2019omsimulator], 
Ecos aims to deliver a higher level of flexibility, extensibility and accessibility through an easy to build and consume
package. In particular, Ecos acts as a successor to the JVM based Vico framework. 


# Future of Ecos

Ecos currently ships with a capable, but simple _fixed_step_ orchestration algorithm. 
The API is designed to be extensible and the goal is to include more advanced orchestration algorithms. 
However, pursuing this should be driven by a clear user need.


# References