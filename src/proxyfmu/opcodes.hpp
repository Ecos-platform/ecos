
#ifndef FUNCTORS_HPP
#define FUNCTORS_HPP

#include <string>

namespace ecos::proxy
{

enum class opcodes
{

    instantiate,
    setup_experiment,
    enter_initialization_mode,
    exit_initialization_mode,

    step,
    terminate,
    reset,
    freeInstance,

    read_int,
    read_real,
    read_string,
    read_bool,

    write_int,
    write_real,
    write_string,
    write_bool

};

inline uint8_t enum_to_int(opcodes op)
{
    return static_cast<uint8_t>(op);
}

inline opcodes int_to_enum(uint8_t i)
{
    return static_cast<opcodes>(i);
}

inline std::string opcode_to_string(opcodes op)
{
    switch (op)
    {
        case opcodes::instantiate: return "instantiate";
        case opcodes::setup_experiment: return "setup_experiment";
        case opcodes::enter_initialization_mode: return "enter_initialization_mode";
        case opcodes::exit_initialization_mode: return "exit_initialization_mode";

        case opcodes::step: return "step";
        case opcodes::terminate: return "terminate";
        case opcodes::reset: return "reset";
        case opcodes::freeInstance: return "freeInstance";

        case opcodes::read_int: return "read_int";
        case opcodes::read_real: return "read_real";
        case opcodes::read_string: return "read_string";
        case opcodes::read_bool: return "read_bool";

        case opcodes::write_int: return "write_int";
        case opcodes::write_real: return "write_real";
        case opcodes::write_string: return "write_string";
        case opcodes::write_bool: return "write_bool";

        default: return "unknown_opcode";
    }
}

} // namespace ecos

#endif // FUNCTORS_HPP
