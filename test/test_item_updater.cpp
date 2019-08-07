#include "item_updater.hpp"
#include "mocked_utils.hpp"

#include <sdbusplus/test/sdbus_mock.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace phosphor::software::updater;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

class TestItemUpdaer : public ::testing::Test
{
  public:
    TestItemUpdaer() :
        mockedUtils(
            reinterpret_cast<const utils::MockedUtils&>(utils::getUtils()))
    {
    }

    ~TestItemUpdaer()
    {
    }

    static constexpr auto dBusPath = "/com/example/software/";
    sdbusplus::SdBusMock sdbusMock;
    std::unique_ptr<ItemUpdater> itemUpdater;
    const utils::MockedUtils& mockedUtils;
};

TEST_F(TestItemUpdaer, ctordtor)
{
    auto mockedBus = sdbusplus::get_mocked_new(&sdbusMock);
    itemUpdater = std::make_unique<ItemUpdater>(mockedBus, dBusPath);
}

TEST_F(TestItemUpdaer, NotCreateObjectOnNotPresent)
{
    constexpr auto psuPath = "/com/exmaple/inventory/psu0";
    constexpr auto service = "com.example.Software.Psu";
    auto mockedBus = sdbusplus::get_mocked_new(&sdbusMock);
    EXPECT_CALL(mockedUtils, getPSUInventoryPath)
        .WillOnce(Return(std::vector<std::string>({psuPath})));
    EXPECT_CALL(mockedUtils, getService(_, StrEq(psuPath), _))
        .WillOnce(Return(service));
    EXPECT_CALL(mockedUtils, getPropertyImpl(_, StrEq(service), StrEq(psuPath),
                                             _, StrEq("Present")))
        .WillOnce(Return(
            std::any(sdbusplus::message::variant<bool>(false)))); // not present

    EXPECT_CALL(sdbusMock, sd_bus_emit_object_added(_, _)).Times(0);
    itemUpdater = std::make_unique<ItemUpdater>(mockedBus, dBusPath);
}
