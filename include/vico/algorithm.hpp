
#ifndef VICO_ALGORITHM_HPP
#define VICO_ALGORITHM_HPP

namespace vico {

struct algorithm {

    void step(double currentTime, double stepSize) = 0;

    virtual ~algorithm() = default;

};

struct fixed_step_algorithm: public algorithm {



};

}

#endif // VICO_ALGORITHM_HPP
