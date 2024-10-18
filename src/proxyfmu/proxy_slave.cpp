
#include "proxy_slave.hpp"

#include "process_helper.hpp"

#include <msgpack.hpp>

#include <chrono>
#include <fstream>
#include <utility>
#include <vector>

#include "proxyfmu/functors.hpp"

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


namespace proxyfmu
{

proxy_slave::proxy_slave(
    const std::filesystem::path& fmuPath,
    const std::string& instanceName,
    fmilibcpp::model_description modelDescription,
    const std::optional<remote_info>& remote)
    : slave(instanceName)
    , modelDescription_(std::move(modelDescription))
{
    int port = -1;
    std::string host;

    if (!remote) {
        host = "localhost";
        std::mutex mtx;
        std::condition_variable cv;
        thread_ = std::thread(&start_process, fmuPath, instanceName, std::ref(port), std::ref(mtx), std::ref(cv));
        std::unique_lock lck(mtx);
        while (port == -1) cv.wait(lck);
    } else {
        host = remote->host();

        TCPClientContext ctx;
        auto client = ctx.connect(host, remote->port());

        std::string data = read_data(fmuPath.string());

        const std::string fmuName = std::filesystem::path(fmuPath).stem().string();

        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, fmuName);
        msgpack::pack(sbuf, instanceName);
        msgpack::pack(sbuf, data);
        sbuf.write(data.c_str(), data.size());

        client->write(sbuf.data(), sbuf.size());

        std::vector<uint8_t> buffer(64);
        const int read = client->read(buffer.data(), buffer.size());

        const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
        oh.get().convert(port);
    }

    if (port == -999) {
        if (thread_.joinable()) thread_.join();
        throw std::runtime_error("[proxyfmu] Unable to bind to external proxy process!");
    }

    client_ = ctx_.connect(host, port);

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::instantiate));
    client_->write(sbuf.data(), sbuf.size());
}

const fmilibcpp::model_description& proxy_slave::get_model_description() const
{
    return modelDescription_;
}

bool proxy_slave::setup_experiment(double start_time, double stop_time, double tolerance)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::setup_experiment));
    msgpack::pack(sbuf, start_time);
    msgpack::pack(sbuf, stop_time);
    msgpack::pack(sbuf, tolerance);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::enter_initialization_mode()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::enter_initialization_mode));
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::exit_initialization_mode()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::exit_initialization_mode));
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::step(double current_time, double step_size)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::step));
    msgpack::pack(sbuf, current_time);
    msgpack::pack(sbuf, step_size);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::terminate()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::terminate));
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::reset()
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::reset));
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    bool status;
    const auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, nullptr);
    return oh.get().convert(status);
}

bool proxy_slave::get_integer(const std::vector<fmilibcpp::value_ref>& vr, std::vector<int>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::read_int));
    msgpack::pack(sbuf, vr);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
        std::cerr << "Error during unpacking: " << e.what() << std::endl;
        return false;
    }

    return status;
}

bool proxy_slave::get_real(const std::vector<fmilibcpp::value_ref>& vr, std::vector<double>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::read_int));
    msgpack::pack(sbuf, vr);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
        std::cerr << "Error during unpacking: " << e.what() << std::endl;
        return false;
    }

    return status;
}

bool proxy_slave::get_string(const std::vector<fmilibcpp::value_ref>& vr, std::vector<std::string>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::read_int));
    msgpack::pack(sbuf, vr);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
        std::cerr << "Error during unpacking: " << e.what() << std::endl;
        return false;
    }

    return status;
}

bool proxy_slave::get_boolean(const std::vector<fmilibcpp::value_ref>& vr, std::vector<bool>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::read_int));
    msgpack::pack(sbuf, vr);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
        std::cerr << "Error during unpacking: " << e.what() << std::endl;
        return false;
    }

    return status;
}

bool proxy_slave::set_integer(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<int>& values)
{
    assert(values.size() == vr.size());

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, enum_to_int(ecos::functors::write_int));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
    msgpack::pack(sbuf, enum_to_int(ecos::functors::write_real));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
    msgpack::pack(sbuf, enum_to_int(ecos::functors::write_string));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
    msgpack::pack(sbuf, enum_to_int(ecos::functors::write_bool));
    msgpack::pack(sbuf, vr);
    msgpack::pack(sbuf, values);
    client_->write(sbuf.data(), sbuf.size());

    std::vector<uint8_t> buffer(32);
    const int read = client_->read(buffer.data(),  buffer.size());

    if (read <= 0) {
        std::cerr << "Failed to read data from client" << std::endl;
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
        if (client_) {
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, enum_to_int(ecos::functors::freeInstance));
            client_->write(sbuf.data(), sbuf.size());
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

} // namespace proxyfmu
