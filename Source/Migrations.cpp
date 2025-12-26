#include "AJAMain.h"

#include <nosSysDevice/nosDeviceSubsystem.h>

#include "AJADevice.h"

namespace nos::aja
{

std::optional<nos::fb::TNode> MigrateChannelNode(nosFbNodePtr node)
{
    auto pluginVersion = node->plugin_version();
    bool needsMigration = !pluginVersion || pluginVersion->major() <= 2 && pluginVersion->minor() < 13;
    if (!needsMigration)
        return std::nullopt;
    fb::TNode cur;
    node->UnPackTo(&cur);
    
    // Try to find suitable device using Device pin
    uint64_t serialNumber = 0;
    for (auto& pin : cur.pins)
    {
        if (pin->name == "Device") {
			if (pin->type_name == "nos.sys.device.DeviceInfo")
			{
                sys::device::TDeviceInfo info = InterpretObjectData<sys::device::TDeviceInfo>(pin->data.data());
                nosDeviceInfo deviceInfoFromPin = sys::device::ConvertDeviceInfo(info);
                nosDeviceId deviceId{};
                uint64_t newDeviceSerial = -1;
                auto res = nosDevice->GetSuitableDevice(&deviceInfoFromPin, &deviceId);
                if (res != NOS_RESULT_SUCCESS)
                {
                    if (deviceInfoFromPin.VendorName != NOS_NAME("None"))
                        nosEngine.LogE("Failed to get suitable device");
                }
                else
                {
                    uint64_t handle{};
                    res = nosDevice->GetDeviceHandle(deviceId, &handle);
                    assert(res == NOS_RESULT_SUCCESS);
                    newDeviceSerial = handle;
                }

                auto device = AJADevice::GetDeviceBySerialNumber(newDeviceSerial);
                if (device)
                    serialNumber = newDeviceSerial;
			}
            else
            {
                const char* oldValue = reinterpret_cast<const char*>(pin->data.data());
                std::string_view oldChannelStr(oldValue, pin->data.size() - 1);
                nosEngine.LogW("Migrating %s: Searching suitable device for '%s'", node->class_name()->c_str(), oldValue);

                auto device = AJADevice::GetDevice(oldChannelStr);
                if (!device)
                {
                    nosEngine.LogE("Failed to find suitable device for '%s'", oldValue);
                    continue;
                }
                nosDeviceInfo info{};
                auto res = nosDevice->GetDeviceInfo(device->GlobalDeviceId, &info);
                sys::device::TDeviceInfo pinData = sys::device::NoneDeviceInfo();
                if (res != NOS_RESULT_FAILED) {
                    pinData = sys::device::ConvertDeviceInfo(info);
                    uint64_t handle{};
                    res = nosDevice->GetDeviceHandle(device->GlobalDeviceId, &handle);
                    assert(res == NOS_RESULT_SUCCESS);
                    serialNumber = handle;
                }

                pin->type_name = "nos.sys.device.DeviceInfo";
                pin->data = nos::Buffer::From(pinData);
            }
        }
    }
	// Delete the ReferenceSource pin if it exists
    // And set the global reference source as the pin's value
    if (serialNumber)
        std::erase_if(cur.pins, [serialNumber](const auto& pin)
            {
                if (pin->name == "ReferenceSource" && pin->type_name == "string") {
                    const char* refValue = InterpretObjectData<char>(pin->data.data());
                    auto device = AJADevice::GetDeviceBySerialNumber(serialNumber);
                    if (!device) {
						nosEngine.LogE("Failed to find device with serial number %llu for ReferenceSource migration", serialNumber);
                        return true;
                    }

                    device->UpdateReferenceSource(refValue, true);
                    return true;
                }
                return false;
            }
        );
    return cur;
}

}