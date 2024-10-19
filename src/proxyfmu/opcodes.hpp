
#ifndef FUNCTORS_HPP
#define FUNCTORS_HPP

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

} // namespace ecos

#endif // FUNCTORS_HPP
