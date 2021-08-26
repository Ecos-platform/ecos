
#ifndef VICO_VISITOR_HPP
#define VICO_VISITOR_HPP

namespace
{

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace

#endif // VICO_VISITOR_HPP
