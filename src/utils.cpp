#include "config.h"

#include "utils.hpp"

#include <openssl/sha.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/log.hpp>

using json = nlohmann::json;

using namespace phosphor::logging;

namespace utils
{

namespace // anonymous
{
constexpr auto MAPPER_BUSNAME = "xyz.openbmc_project.ObjectMapper";
constexpr auto MAPPER_PATH = "/xyz/openbmc_project/object_mapper";
constexpr auto MAPPER_INTERFACE = "xyz.openbmc_project.ObjectMapper";
} // namespace

namespace details
{

json loadFromFile(const char* path)
{
    std::ifstream ifs(path);
    if (!ifs.good())
    {
        log<level::ERR>("Unable to open file", entry("PATH=%s", path));
        return nullptr;
    }
    auto data = json::parse(ifs, nullptr, false);
    if (data.is_discarded())
    {
        log<level::ERR>("Failed to parse json", entry("PATH=%s", path));
        return nullptr;
    }
    return data;
}
} // namespace details

const UtilsInterface& getUtils()
{
    static Utils utils;
    return utils;
}

std::string Utils::getService(sdbusplus::bus::bus& bus, const char* path,
                              const char* interface) const
{
    auto mapper = bus.new_method_call(MAPPER_BUSNAME, MAPPER_PATH,
                                      MAPPER_INTERFACE, "GetObject");

    mapper.append(path, std::vector<std::string>({interface}));
    try
    {
        auto mapperResponseMsg = bus.call(mapper);

        std::vector<std::pair<std::string, std::vector<std::string>>>
            mapperResponse;
        mapperResponseMsg.read(mapperResponse);
        if (mapperResponse.empty())
        {
            log<level::ERR>("Error reading mapper response");
            throw std::runtime_error("Error reading mapper response");
        }
        if (mapperResponse.size() < 1)
        {
            return "";
        }
        return mapperResponse[0].first;
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("Mapper call failed", entry("METHOD=%d", "GetObject"),
                        entry("PATH=%s", path),
                        entry("INTERFACE=%s", interface));
        throw std::runtime_error("Mapper call failed");
    }
}

std::vector<std::string> Utils::getPSUInventoryPath() const
{
    auto data = details::loadFromFile(PSU_JSON_CONFIG);

    if (data == nullptr)
    {
        return {};
    }

    if (data.find("PSU_INVENTORY_PATH") == data.end())
    {
        log<level::WARNING>("Unable to find PSU_INVENTORY_PATH");
    }

    return data["PSU_INVENTORY_PATH"];
}

std::string Utils::getVersionId(const std::string& version) const
{
    if (version.empty())
    {
        log<level::ERR>("Error version is empty");
        return {};
    }

    unsigned char digest[SHA512_DIGEST_LENGTH];
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    SHA512_Update(&ctx, version.c_str(), strlen(version.c_str()));
    SHA512_Final(digest, &ctx);
    char mdString[SHA512_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    {
        snprintf(&mdString[i * 2], 3, "%02x", (unsigned int)digest[i]);
    }

    // Only need 8 hex digits.
    std::string hexId = std::string(mdString);
    return (hexId.substr(0, 8));
}

std::any Utils::getPropertyImpl(sdbusplus::bus::bus& bus, const char* service,
                                const char* path, const char* interface,
                                const char* propertyName) const
{
    auto method = bus.new_method_call(service, path,
                                      "org.freedesktop.DBus.Properties", "Get");
    method.append(interface, propertyName);
    try
    {
        sdbusplus::message::variant<std::string, bool> value{};
        auto reply = bus.call(method);
        reply.read(value);
        return std::any(value);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        log<level::ERR>("GetProperty call failed", entry("PATH=%s", path),
                        entry("INTERFACE=%s", interface),
                        entry("PROPERTY=%s", propertyName));
        throw std::runtime_error("GetProperty call failed");
    }
}

} // namespace utils
