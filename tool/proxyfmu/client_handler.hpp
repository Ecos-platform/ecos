
#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include "fmilibcpp/fmu.hpp"
#include "fmilibcpp/slave.hpp"
#include "proxyfmu/opcodes.hpp"
#include "simple_socket/TCPSocket.hpp"
#include <msgpack.hpp>
#include <spdlog/spdlog.h>

inline void sendStatus(simple_socket::SimpleConnection& conn, bool status)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, status);
    conn.write(sbuf.data(), sbuf.size());
}

template<typename T>
void sendStatusAndValues(simple_socket::SimpleConnection& conn, bool status, const std::vector<T>& values)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, status);
    msgpack::pack(sbuf, values);
    conn.write(sbuf.data(), sbuf.size());
}

inline void client_handler(std::unique_ptr<simple_socket::SimpleConnection> conn, const std::string& fmu, const std::string& instanceName)
{

    std::unique_ptr<fmilibcpp::slave> slave;
    std::vector<uint8_t> buffer(1024 * 32);
    ecos::proxy::opcodes op{ecos::proxy::opcodes::NONE};
    try {
        bool stop{false};
        while (!stop) {
            const int read = conn->read(buffer);

            uint8_t func;
            std::size_t offset = 0;
            try {
                auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                oh.get().convert(func);
            } catch (const std::exception& e) {
                spdlog::error("Exception on unpack: {}", e.what());
                sendStatus(*conn, false);
                continue;
            }

            op = ecos::proxy::int_to_enum(func);
            spdlog::trace("Got opcode: {}", opcode_to_string(op));
            switch (op) {
                case ecos::proxy::opcodes::instantiate: {
                    auto model = fmilibcpp::loadFmu(fmu);
                    slave = model->new_instance(instanceName);
                    uint8_t token;
                    conn->write(&token, 1);
                } break;
                case ecos::proxy::opcodes::setup_experiment: {

                    double startTime;
                    double endTime;
                    double tolerance;
                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(startTime);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(endTime);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(tolerance);

                    const auto status = slave->setup_experiment(startTime, endTime, tolerance);
                    sendStatus(*conn, status);

                } break;
                case ecos::proxy::opcodes::enter_initialization_mode: {
                    const auto status = slave->enter_initialization_mode();
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::exit_initialization_mode: {
                    const auto status = slave->exit_initialization_mode();
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::step: {
                    double currentTime;
                    double stepSize;
                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(currentTime);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(stepSize);
                    const auto status = slave->step(currentTime, stepSize);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::terminate: {
                    const auto status = slave->terminate();
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::freeInstance: {
                    slave->freeInstance();
                    stop = true;
                    continue;
                }
                case ecos::proxy::opcodes::read_int: {
                    std::vector<fmilibcpp::value_ref> vr;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);

                    std::vector<int> values(vr.size());
                    const auto status = slave->get_integer(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::read_real: {
                    std::vector<fmilibcpp::value_ref> vr;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);

                    std::vector<double> values(vr.size());
                    const auto status = slave->get_real(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::read_string: {
                    std::vector<fmilibcpp::value_ref> vr;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);

                    std::vector<std::string> values(vr.size());
                    const auto status = slave->get_string(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::read_bool: {
                    std::vector<fmilibcpp::value_ref> vr;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);

                    std::vector<bool> values(vr.size());
                    const auto status = slave->get_boolean(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::write_int: {
                    std::vector<fmilibcpp::value_ref> vr;
                    std::vector<int> values;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(values);

                    const auto status = slave->set_integer(vr, values);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::write_real: {
                    std::vector<fmilibcpp::value_ref> vr;
                    std::vector<double> values;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(values);

                    const auto status = slave->set_real(vr, values);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::write_string: {
                    std::vector<fmilibcpp::value_ref> vr;
                    std::vector<std::string> values;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(values);

                    const auto status = slave->set_string(vr, values);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::write_bool: {
                    std::vector<fmilibcpp::value_ref> vr;
                    std::vector<bool> values;

                    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(vr);
                    oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
                    oh.get().convert(values);

                    const auto status = slave->set_boolean(vr, values);
                    sendStatus(*conn, status);
                } break;
                default: {
                    spdlog::error("Unknown command: {}", func);
                    sendStatus(*conn, false);
                } break;
            }
        }
    } catch (const std::exception& ex) {
        spdlog::warn("Exception in client handler: {}. Last opcode={}", ex.what(), opcode_to_string(op));
    }

    spdlog::info("Client handler exit, last opcode={}", opcode_to_string(op));
}


#endif // CLIENT_HANDLER_HPP
