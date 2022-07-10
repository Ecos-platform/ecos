
#include "vico/ssp/ssp_loader.hpp"

#include "ssp/ssp.hpp"

#include "vico/fmi/fmi_model.hpp"
#include "vico/fmi/proxy/proxy_model.hpp"

#include "vico/model_resolver.hpp"

using namespace vico;

namespace
{

//class proxy_model_ssp: public proxy_model
//{
//
//public:
//    proxy_model_ssp(const std::filesystem::path& fmuPath, std::shared_ptr<ssp::temp_dir> tempDir)
//        : proxy_model(fmuPath), tempDir_(std::move(tempDir)) {}
//
//private:
//    std::shared_ptr<ssp::temp_dir> tempDir_;
//};
//
//std::unique_ptr<model> resolve_(
//    const ssp::SystemStructureDescription& desc,
//    const std::string& source)
//{
//    if (source.rfind("proxyfmu", 0) == 0) {
//        const auto find = source.find("file=", 0);
//        if (find == std::string::npos) {
//            throw std::runtime_error("proxyfmu source missing file= component..");
//        }
//        const auto fmuFile = desc.file(source.substr(find + 5));
//        return std::make_unique<proxy_model_ssp>(fmuFile, desc.get_temp_dir());
//    } else {
//        const auto fmuFile = desc.file(source);
//        return std::make_unique<fmi_model>(fmuFile);
//    }
//}

class ssp_based_simulation_structure: public simulation_structure
{
public:
    explicit ssp_based_simulation_structure(const std::filesystem::path& path)
        : desc_(path) {

        const auto& system = desc_.system;
        const auto& parameterSets = system.elements.parameterSets;
        const auto& components = system.elements.components;
        const auto& connections = system.connections;

        auto resolver = default_model_resolver();
        for (const auto& [name, component] : components) {
            auto model = resolver->resolve(desc_.dir(), component.source);
            add_model(name, model);
        }

        for (const auto& connection : connections) {
            const auto& startComponent = components.at(connection.startElement);
            const auto& startConnector = startComponent.connectors.at(connection.startConnector);
            const auto& endComponent = components.at(connection.endElement);
            const auto& endConnector = endComponent.connectors.at(connection.endConnector);
            const variable_identifier source(connection.startElement, connection.startConnector);
            const variable_identifier sink(connection.endElement, connection.endConnector);

            if (!(startConnector.type == endConnector.type)) {
                throw std::runtime_error("Incompatible connector types!");
            }

            const auto typeName = startConnector.type.typeName();
            if (typeName == "Real") {
                std::optional<std::function<double(double)>> f;
                if (connection.linearTransformation) {
                    f = [connection](double value) {
                        return value + connection.linearTransformation->offset * connection.linearTransformation->factor;
                    };
                }
                make_connection<double>(source, sink, f);
            } else if (typeName == "Integer") {
                make_connection<int>(source, sink);
            } else if (typeName == "Boolean") {
                make_connection<bool>(source, sink);
            } else if (typeName == "String") {
                make_connection<std::string>(source, sink);
            }
        }

        for (const auto& [parameterSetName, sets] : parameterSets) {
            std::map<variable_identifier, std::variant<double, int, bool, std::string>> map;
            for (const auto& [component, parameters] : sets) {
                for (const auto& p : parameters) {
                    variable_identifier v{component.name, p.name};
                    map[v] = p.type.value;
                }
            }
            if (!map.empty()) {
                add_parameter_set(parameterSetName, map);
            }
        }
    }
    
private:
    ssp::SystemStructureDescription desc_;
};

} // namespace

std::unique_ptr<simulation_structure> vico::load_ssp(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("No such file: '" + std::filesystem::absolute(path).string() + "'");
    }

    return std::make_unique<ssp_based_simulation_structure>(path);
}