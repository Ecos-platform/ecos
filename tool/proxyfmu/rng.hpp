
#ifndef ECOS_FIXED_RANGE_RANDOM_GENERATOR_HPP
#define ECOS_FIXED_RANGE_RANDOM_GENERATOR_HPP

#include <random>

class rng
{

private:
    std::mt19937 mt_;
    std::uniform_int_distribution<int> dist_;

public:
    rng(int min, int max)
        : mt_(std::random_device()())
        , dist_(min, max)
    { }

    int next()
    {
        return dist_(mt_);
    }
};

#endif // ECOS_FIXED_RANGE_RANDOM_GENERATOR_HPP
