
#include "proxy_slave.hpp"

#include "../../cmake-build-debug-wsl/_deps/simplesocket-src/include/simple_socket/UnixDomainSocket.hpp"
#include "process_helper.hpp"

#include <ecos/logger/logger.hpp>

#include "proxyfmu/opcodes.hpp"
#include "simple_socket/util/byte_conversion.hpp"
#include <msgpack.hpp>

#include <chrono>
#include <fstream>
#include <utility>
#include <vector>

using namespace simple_socket;

namespace
{

std::string read_data(std::string const& fileName)
{
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read file: " + fileName);
    }

    return {buffer.begin(), buffer.end()};
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
        if (bind.empty()) throw std::runtime_error("Unable to bind");
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

        const std::string data = read_data(fmuPath.string());
        const std::string fmuName = std::filesystem::path(fmuPath).stem().string();

        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, fmuName);
        msgpack::pack(sbuf, instanceName);
        msgpack::pack(sbuf, data);
        sbuf.write(data.c_str(), data.size());

        const auto msgLen = sbuf.size();
        bootClient->write(encode_uint32(msgLen));
        bootClient->write(sbuf.data(), sbuf.size());

        std::vector<uint8_t> buffer(32);
        const int read = bootClient->read(buffer.data(), buffer.size());

        uint16_t port;
        const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
        oh.get().convert(port);

        client_ = ctx_->connect(remote->host() + ":" + std::to_string(port));
    }

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::instantiate));
    client_->write(sbuf.data(), sbuf.size());

    uint8_t token;
    client_->readExact(&token, 1);
}

const fmilibcpp::model_description& proxy_slave::get_model_description() const
{
    return modelDescription_;
}

bool proxy_slave::setup_experiment(double start_time, double stop_time, double tolerance)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::setup_experiment));
    msgpack::pack(sbuf, start_time);
    msgpack::pack(sbuf, stop_time);
    msgpack::pack(sbuf, tolerance);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }
    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    bool status;
    std::size_t offset = 0;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
    oh.get().convert(status);

    spdlog::info("Status={}", status);
    return status;
}

bool proxy_slave::enter_initialization_mode()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::enter_initialization_mode));
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    bool status;
    std::size_t offset = 0;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
    return oh.get().convert(status);
}

bool proxy_slave::exit_initialization_mode()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::exit_initialization_mode));
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    bool status;
    std::size_t offset = 0;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
    return oh.get().convert(status);
}

bool proxy_slave::step(double current_time, double step_size)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::step));
    msgpack::pack(sbuf, current_time);
    msgpack::pack(sbuf, step_size);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    bool status;
    std::size_t offset = 0;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
    return oh.get().convert(status);
}

bool proxy_slave::terminate()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::terminate));
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::reset()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::reset));
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(), buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::get_integer(const std::vector<fmilibcpp::value_ref>& vr, std::vector<int>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::read_int));
    msgpack::pack(sbuf, vr);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_integer] Failed to read data from client");
        return false;
    }

    bool status;

    try {
        std::size_t offset = 0;
        auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(status);

        oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(values);
    } catch (const std::exception& e) {
        log::err("[get_integer] Error during unpacking: {}", e.what());
        return false;
    }

    return status;
}

bool proxy_slave::get_real(const std::vector<fmilibcpp::value_ref>& vr, std::vector<double>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::read_real));
    msgpack::pack(sbuf, vr);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_real] Failed to read data from client");
        return false;
    }

    bool status;

    try {
        std::size_t offset = 0;
        auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(status);

        oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(values);
    } catch (const std::exception& e) {
        log::err("[get_real] Error during unpacking: {}", e.what());
        return false;
    }

    return status;
}

bool proxy_slave::get_string(const std::vector<fmilibcpp::value_ref>& vr, std::vector<std::string>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::read_string));
    msgpack::pack(sbuf, vr);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_string] Failed to read data from client");
        return false;
    }

    bool status;
    std::size_t offset = 0;

    try {
        auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(status);

        oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(values);
    } catch (const std::exception& e) {
        log::err("[get_string] Error during unpacking: {}",  e.what());
        return false;
    }

    return status;
}

bool proxy_slave::get_boolean(const std::vector<fmilibcpp::value_ref>& vr, std::vector<bool>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::read_bool));
    msgpack::pack(sbuf, vr);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    std::vector<uint8_t> buffer(512);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[get_boolean] Failed to read data from client");
        return false;
    }

    bool status;
    std::size_t offset = 0;

    try {
        auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(status);

        oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
        oh.get().convert(values);
    } catch (const std::exception& e) {
        log::err("[get_boolean] Error during unpacking: {}", e.what());
        return false;
    }

    return status;
}

bool proxy_slave::set_integer(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<int>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::write_int));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512*2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_integer] Failed to read data from client");
        return false;
    }

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::set_real(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<double>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::write_real));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512*2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_real] Failed to read data from client");
        return false;
    }

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::set_string(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<std::string>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::write_string));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512*2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_string] Failed to read data from client");
        return false;
    }

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::set_boolean(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<bool>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(opcodes::write_bool));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    if (!client_->write(sbuf.data(), sbuf.size())) {
        return false;
    }

    static std::vector<uint8_t> buffer(512*2);
    const int read = client_->read(buffer.data(), buffer.size());

    if (read <= 0) {
        log::err("[set_boolean] Failed to read data from client");
        return false;
    }

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

void proxy_slave::freeInstance()
{
    if (!freed) {
        freed = true;
        log::debug("Shutting down proxy for '{}::{}'", modelDescription_.modelName, instanceName);
        if (client_) {
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, enum_to_int(opcodes::freeInstance));
            client_->write(sbuf.data(), sbuf.size());
        }
        if (thread_.joinable()) {
            thread_.join();
        }
        log::debug("done..");
        // communication with parent process
        std::cout << "[proxyfmu] freed";
    }
}

proxy_slave::~proxy_slave()
{
    proxy_slave::freeInstance();
}

} // namespace proxyfmu
