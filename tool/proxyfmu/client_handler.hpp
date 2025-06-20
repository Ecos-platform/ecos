
#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include "fmilibcpp/fmu.hpp"
#include "fmilibcpp/slave.hpp"

#include "proxyfmu/opcodes.hpp"
#include "simple_socket/SimpleConnection.hpp"
#include <flatbuffers/flexbuffers.h>
#include <spdlog/spdlog.h>

inline void sendStatus(simple_socket::SimpleConnection& conn, bool status)
{
    flexbuffers::Builder fbb;
    fbb.Bool(status);
    fbb.Finish();
    conn.write(fbb.GetBuffer());
}

template<typename T>
struct is_bool : std::false_type
{
};

template<>
struct is_bool<bool> : std::true_type
{
};

template<typename T>
struct is_string : std::false_type
{
};

template<>
struct is_string<std::string> : std::true_type
{
};

template<typename T>
void sendStatusAndValues(simple_socket::SimpleConnection& conn, bool status, const std::vector<T>& values)
{
    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Bool(status);
        if constexpr (is_bool<T>::value) {
            // If T is std::vector<bool>, pack each value as a boolean
            fbb.Vector([&] {
                for (const bool value : values) {
                    fbb.Bool(value);
                }
            });
        } else if constexpr (is_string<T>::value) {
            // If T is std::string, pack each string
            fbb.Vector([&] {
                for (const auto& value : values) {
                    fbb.String(value);
                }
            });
        } else {
            fbb.Vector(values);
        }
    });

    fbb.Finish();
    conn.write(fbb.GetBuffer());
}

inline void client_handler(std::unique_ptr<simple_socket::SimpleConnection> conn, const std::string& fmu, const std::string& instanceName)
{

    std::unique_ptr<fmilibcpp::slave> slave;
    std::vector<uint8_t> buffer(1024 * 32);
    auto op{ecos::proxy::opcodes::NONE};
    try {
        bool stop{false};
        while (!stop) {
            const int read = conn->read(buffer);

            auto root = flexbuffers::GetRoot(buffer.data(), read).AsVector();

            uint16_t arg{0};
            uint8_t func = root[arg++].AsUInt8();
            op = ecos::proxy::int_to_enum(func);
            spdlog::trace("Got opcode: {}", opcode_to_string(op));
            switch (op) {
                case ecos::proxy::opcodes::instantiate: {
                    auto model = fmilibcpp::loadFmu(fmu);
                    slave = model->new_instance(instanceName);
                    uint8_t token;
                    conn->write(&token, 1);
                } break;
                case ecos::proxy::opcodes::enter_initialization_mode: {
                    double startTime = root[arg++].AsDouble();
                    double endTime = root[arg++].AsDouble();
                    double tolerance = root[arg++].AsDouble();

                    const auto status = slave->enter_initialization_mode(startTime, endTime, tolerance);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::exit_initialization_mode: {
                    const auto status = slave->exit_initialization_mode();
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::step: {
                    double currentTime = root[arg++].AsDouble();
                    double stepSize = root[arg++].AsDouble();

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
                    const auto flexVr = root[arg++].AsTypedVector();
                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }

                    std::vector<int32_t> values(vr.size());
                    const auto status = slave->get_integer(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::read_real: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }

                    std::vector<double> values(vr.size());
                    const auto status = slave->get_real(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::read_string: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }

                    std::vector<std::string> values(vr.size());
                    const auto status = slave->get_string(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::read_bool: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }

                    std::vector<bool> values(vr.size());
                    const auto status = slave->get_boolean(vr, values);

                    sendStatusAndValues(*conn, status, values);
                } break;
                case ecos::proxy::opcodes::write_int: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    const auto flexValues = root[arg++].AsTypedVector();

                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }
                    std::vector<int32_t> values(vr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        values[i] = flexValues[i].AsInt32();
                    }

                    const auto status = slave->set_integer(vr, values);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::write_real: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    const auto flexValues = root[arg++].AsTypedVector();

                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }
                    std::vector<double> values(vr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        values[i] = flexValues[i].AsDouble();
                    }

                    const auto status = slave->set_real(vr, values);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::write_string: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    const auto flexValues = root[arg++].AsVector();

                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }
                    std::vector<std::string> values(vr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        values[i] = flexValues[i].AsString().str();
                    }

                    const auto status = slave->set_string(vr, values);
                    sendStatus(*conn, status);
                } break;
                case ecos::proxy::opcodes::write_bool: {
                    const auto flexVr = root[arg++].AsTypedVector();
                    const auto flexValues = root[arg++].AsVector();

                    std::vector<fmilibcpp::value_ref> vr(flexVr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        vr[i] = flexVr[i].AsUInt32();
                    }
                    std::vector<bool> values(vr.size());
                    for (auto i = 0; i < flexVr.size(); i++) {
                        values[i] = flexValues[i].AsBool();
                    }

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
