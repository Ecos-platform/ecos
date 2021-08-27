
#ifndef VICO_SYSTEM_HPP
#define VICO_SYSTEM_HPP

#include <string>
#include <unordered_map>

namespace vico
{

class system
{

public:
    virtual void init() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    virtual ~system() = default;
};


} // namespace vico

#endif // VICO_SYSTEM_HPP
