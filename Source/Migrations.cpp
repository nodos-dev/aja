#include "AJAMain.h"

#include <nosDeviceSubsystem/nosDeviceSubsystem.h>

#include "AJADevice.h"

namespace nos::aja
{

std::optional<nos::fb::TNode> MigrateChannelNode(nosFbNodePtr node)
{
    auto pluginVersion = node->plugin_version();
    bool needsMigration = !pluginVersion || pluginVersion->major() <= 2 && pluginVersion->minor() < 3;
    if (!needsMigration)
        return std::nullopt;
    fb::TNode cur;
    node->UnPackTo(&cur);
    
    for (auto& pin : cur.pins)
    {
        if (pin->name == "Device" && pin->type_name != "nos.sys.device.DeviceInfo")
        {
            const char* oldValue = reinterpret_cast<const char*>(pin->data.data());
			std::string_view oldChannelStr (oldValue, pin->data.size() - 1);
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
            if (res != NOS_RESULT_FAILED)
                pinData = sys::device::ConvertDeviceInfo(info);

            pin->type_name = "nos.sys.device.DeviceInfo";
            pin->data = nos::Buffer::From(pinData);
        }
    }
    return cur;
}

}