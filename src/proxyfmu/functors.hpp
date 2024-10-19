
#ifndef FUNCTORS_HPP
#define FUNCTORS_HPP

namespace ecos
{

enum class functors
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

inline uint8_t enum_to_int(functors f)
{
    return static_cast<uint8_t>(f);
}

inline functors int_to_enum(uint8_t i)
{
    return static_cast<functors>(i);
}

} // namespace ecos

#endif // FUNCTORS_HPP
