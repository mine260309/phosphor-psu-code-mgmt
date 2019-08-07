#pragma once

#include <any>
#include <sdbusplus/bus.hpp>
#include <string>
#include <vector>

namespace utils
{

class UtilsInterface;

/**
 * @brief Get the implementation of UtilsInterface
 */
const UtilsInterface& getUtils();

/**
 * @brief Get PSU inventory object path from config
 */
std::vector<std::string> getPSUInventoryPath();

/** @brief Get service name from object path and interface
 *
 * @param[in] bus          - The Dbus bus object
 * @param[in] path         - The Dbus object path
 * @param[in] interface    - The Dbus interface
 *
 * @return The name of the service
 */
std::string getService(sdbusplus::bus::bus& bus, const char* path,
                       const char* interface);

/** @brief The template function to get property from the requested dbus path
 *
 * @param[in] bus          - The Dbus bus object
 * @param[in] service      - The Dbus service name
 * @param[in] path         - The Dbus object path
 * @param[in] interface    - The Dbus interface
 * @param[in] propertyName - The property name to get
 *
 * @return The value of the property
 */
template <typename T>
T getProperty(sdbusplus::bus::bus& bus, const char* service, const char* path,
              const char* interface, const char* propertyName);

/**
 * @brief Calculate the version id from the version string.
 *
 * @details The version id is a unique 8 hexadecimal digit id
 *          calculated from the version string.
 *
 * @param[in] version - The image version string (e.g. v1.99.10-19).
 *
 * @return The id.
 */
std::string getVersionId(const std::string& version);

/**
 * @brief The interface for utils
 */
class UtilsInterface
{
  public:
    virtual ~UtilsInterface() = default;

    virtual std::vector<std::string> getPSUInventoryPath() const = 0;

    virtual std::string getService(sdbusplus::bus::bus& bus, const char* path,
                                   const char* interface) const = 0;

    virtual std::string getVersionId(const std::string& version) const = 0;

    virtual std::any getPropertyImpl(sdbusplus::bus::bus& bus,
                                     const char* service, const char* path,
                                     const char* interface,
                                     const char* propertyName) const = 0;

    template <typename T>
    T getProperty(sdbusplus::bus::bus& bus, const char* service,
                  const char* path, const char* interface,
                  const char* propertyName) const
    {
        std::any result =
            getPropertyImpl(bus, service, path, interface, propertyName);
        sdbusplus::message::variant<T> value =
            std::any_cast<sdbusplus::message::variant<T>>(result);
        return sdbusplus::message::variant_ns::get<T>(value);
    }
};

class Utils : public UtilsInterface
{
  public:
    std::vector<std::string> getPSUInventoryPath() const override;

    std::string getService(sdbusplus::bus::bus& bus, const char* path,
                           const char* interface) const override;

    std::string getVersionId(const std::string& version) const override;

    std::any getPropertyImpl(sdbusplus::bus::bus& bus, const char* service,
                             const char* path, const char* interface,
                             const char* propertyName) const override;
};

inline std::string getService(sdbusplus::bus::bus& bus, const char* path,
                              const char* interface)
{
    return getUtils().getService(bus, path, interface);
}

inline std::vector<std::string> getPSUInventoryPath()
{
    return getUtils().getPSUInventoryPath();
}

inline std::string getVersionId(const std::string& version)
{
    return getUtils().getVersionId(version);
}

template <typename T>
T getProperty(sdbusplus::bus::bus& bus, const char* service, const char* path,
              const char* interface, const char* propertyName)
{
    return getUtils().getProperty<T>(bus, service, path, interface,
                                     propertyName);
}

} // namespace utils
