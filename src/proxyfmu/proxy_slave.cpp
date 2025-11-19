
#include "proxy_slave.hpp"

#include "process_helper.hpp"

#include <ecos/logger/logger.hpp>

#include "proxyfmu/opcodes.hpp"
#include "simple_socket/TCPSocket.hpp"
#include "simple_socket/UnixDomainSocket.hpp"
#include "simple_socket/util/byte_conversion.hpp"
#include <flatbuffers/flexbuffers.h>

#include <chrono>
#include <fstream>
#include <utility>
#include <vector>

using namespace simple_socket;

namespace
{

std::vector<uint8_t> read_data(std::string const& fileName)
{
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + fileName);
    }

    return buffer;
}

} // namespace


namespace ecos::proxy
{

proxy_slave::proxy_slave(
    const std::filesystem::path& fmuPath,
    const std::string& instanceName,
    fmilibcpp::model_description modelDescription,
    const std::optional<remote_info>& remote)
    : slave(instanceName)
    , modelDescription_(std::move(modelDescription))
{

    if (!remote) {

        std::promise<std::string> bind_promise;
        thread_ = std::thread(&start_process, fmuPath, instanceName, std::ref(bind_promise), true);

        const auto bind = bind_promise.get_future().get();
        if (bind.empty()) {
            thread_.detach();
            throw std::runtime_error("Unable to create/bind proxyfmu process!");
        }
        ctx_ = std::make_unique<UnixDomainClientContext>();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        client_ = ctx_->connect(bind);

    } else {

        std::string address = remote->host() + ":" + std::to_string(remote->port());

        ctx_ = std::make_unique<TCPClientContext>();
        auto bootClient = ctx_->connect(address);
        if (!bootClient) {
            throw std::runtime_error("Failed to connect to: " + address);
        }

        const auto data = read_data(fmuPath.string());
        const std::string fmuName = std::filesystem::path(fmuPath).stem().string();

        flexbuffers::Builder fbb;
        fbb.Vector([&]() {
            fbb.String(fmuName);
            fbb.String(instanceName);
            fbb.Blob(data); // no key here in a vector
        });
        fbb.Finish();

        const auto msgLen = fbb.GetSize();
        bootClient->write(encode_uint32(msgLen));
        bootClient->write(fbb.GetBuffer());

        std::vector<uint8_t> buffer(32);
        const int read = bootClient->read(buffer.data(), buffer.size());

        uint16_t port;
        auto root = flexbuffers::GetRoot(buffer.data(), read).AsInt16();
        port = static_cast<uint16_t>(root);

        client_ = ctx_->connect(remote->host() + ":" + std::to_string(port));
    }

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::instantiate));
    });
    fbb.Finish();
    client_->write(fbb.GetBuffer());

    uint8_t token;
    client_->readExact(&token, 1);
}

const fmilibcpp::model_description& proxy_slave::get_model_description() const
{
    return modelDescription_;
}

void proxy_slave::set_debug_logging(bool flag)
{
    // TODO
}

bool proxy_slave::enter_initialization_mode(double start_time, double stop_time, double tolerance)
{
    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::enter_initialization_mode));
        fbb.Double(start_time);
        fbb.Double(stop_time);
        fbb.Double(tolerance);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::exit_initialization_mode()
{
    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::exit_initialization_mode));
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::step(double current_time, double step_size)
{
    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::step));
        fbb.Double(current_time);
        fbb.Double(step_size);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::terminate()
{
    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::terminate));
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::reset()
{
    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::reset));
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::get_integer(const std::vector<fmilibcpp::value_ref>& vr, std::vector<int32_t>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::read_int));
        fbb.Vector(vr);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_integer] Failed to read data from client");
        return false;
    }

    const auto root = flexbuffers::GetRoot(buffer.data(), read).AsVector();
    const bool status = root[0].AsBool();
    if (!status) return false;
    const auto flexValues = root[1].AsTypedVector();
    for (auto i = 0; i < flexValues.size(); i++) {
        values[i] = flexValues[i].AsInt32();
    }

    return true;
}

bool proxy_slave::get_real(const std::vector<fmilibcpp::value_ref>& vr, std::vector<double>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::read_real));
        fbb.Vector(vr);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_real] Failed to read data from client");
        return false;
    }

    const auto root = flexbuffers::GetRoot(buffer.data(), read).AsVector();
    const bool status = root[0].AsBool();
    if (!status) return false;
    const auto flexValues = root[1].AsTypedVector();
    for (auto i = 0; i < flexValues.size(); i++) {
        values[i] = flexValues[i].AsDouble();
    }

    return true;
}

bool proxy_slave::get_string(const std::vector<fmilibcpp::value_ref>& vr, std::vector<std::string>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::read_string));
        fbb.Vector(vr);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_string] Failed to read data from client");
        return false;
    }

    const auto root = flexbuffers::GetRoot(buffer.data(), read).AsVector();
    const bool status = root[0].AsBool();
    if (!status) return false;
    const auto flexValues = root[1].AsVector();
    for (auto i = 0; i < flexValues.size(); i++) {
        values[i] = flexValues[i].AsString().str();
    }

    return true;
}

bool proxy_slave::get_boolean(const std::vector<fmilibcpp::value_ref>& vr, std::vector<bool>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::read_bool));
        fbb.Vector(vr);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_boolean] Failed to read data from client");
        return false;
    }

    const auto root = flexbuffers::GetRoot(buffer.data(), read).AsVector();
    const bool status = root[0].AsBool();
    if (!status) return false;
    const auto flexValues = root[1].AsVector();
    for (auto i = 0; i < flexValues.size(); i++) {
        values[i] = flexValues[i].AsBool();
    }

    return true;
}

bool proxy_slave::set_integer(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<int32_t>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::write_int));
        fbb.Vector(vr);
        fbb.Vector(values);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512 * 2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_integer] Failed to read data from client");
        return false;
    }

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::set_real(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<double>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::write_real));
        fbb.Vector(vr);
        fbb.Vector(values);
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512 * 2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_real] Failed to read data from client");
        return false;
    }

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::set_string(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<std::string>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::write_string));
        fbb.Vector(vr);
        fbb.Vector([&] {
            for (const std::string& value : values) {
                fbb.String(value);
            }
        });
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512 * 2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_string] Failed to read data from client");
        return false;
    }

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

bool proxy_slave::set_boolean(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<bool>& values)
{
    assert(values.size() == vr.size());

    flexbuffers::Builder fbb;
    fbb.Vector([&] {
        fbb.Int(enum_to_int(opcodes::write_bool));
        fbb.Vector(vr);
        fbb.Vector([&] {
            for (const bool value : values) {
                fbb.Bool(value);
            }
        });
    });
    fbb.Finish();
    if (!client_->write(fbb.GetBuffer())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512 * 2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_boolean] Failed to read data from client");
        return false;
    }

    const bool status = flexbuffers::GetRoot(buffer.data(), read).AsBool();
    return status;
}

void proxy_slave::freeInstance()
{
    if (!freed) {
        freed = true;
        log::debug("Shutting down proxy for '{}::{}'", modelDescription_.modelName, instanceName);
        if (client_) {
            flexbuffers::Builder fbb;
            fbb.Vector([&] {
                fbb.Int(enum_to_int(opcodes::freeInstance));
            });
            fbb.Finish();
            client_->write(fbb.GetBuffer());
        }
        if (thread_.joinable()) {
            thread_.join();
        }
    }
}

proxy_slave::~proxy_slave()
{
    proxy_slave::freeInstance();
}

} // namespace ecos::proxy
