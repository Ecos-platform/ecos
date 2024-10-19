
#include <catch2/catch_test_macros.hpp>

#include "fmilibcpp/fmi1/fmi1_fmu.hpp"
#include "proxyfmu/opcodes.hpp"
#include <msgpack.hpp>

TEST_CASE("msgpack 1", "[msgpack]")
{

    float start_time = 0;
    float stop_time = 1;
    float tolerance = 0.01;

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::proxy::opcodes::setup_experiment));
    msgpack::pack(sbuf, start_time);
    msgpack::pack(sbuf, stop_time);
    msgpack::pack(sbuf, tolerance);

    int f;
    float f1, f2, f3;
    std::size_t offset = 0;
    auto oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f);
    oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f1);
    oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f2);
    oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f3);

    CHECK(f == enum_to_int(ecos::proxy::opcodes::setup_experiment));
    CHECK(f1 == start_time);
    CHECK(f2 == stop_time);
    CHECK(f3 == tolerance);
}

TEST_CASE("msgpack 2", "[msgpack]")
{

    std::vector<fmilibcpp::value_ref> vr{1, 2, 3};
    std::vector<double> values = {9.1, 10.2, 11.3};

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::proxy::opcodes::write_real));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);

    int f;
    std::vector<fmilibcpp::value_ref> f1;
    std::vector<double> f2;
    std::size_t offset = 0;
    auto oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f);
    oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f1);
    oh = msgpack::unpack(sbuf.data(), sbuf.size(), offset);
    oh.get().convert(f2);

    CHECK(f == enum_to_int(ecos::proxy::opcodes::write_real));
    CHECK(f1 == vr);
    CHECK(f2 == values);
}
