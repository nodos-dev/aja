// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#include <Nodos/Plugin.hpp>

#include "AJA_generated.h"
#include "AJADevice.h"
#include "AJAMain.h"

#include <nosSync/nosSync.h>

#include <ctime>

namespace nos::aja
{

NOS_REGISTER_NAME(VBLFailed)

nosResult WaitVBLEvent(void* ctx, nosWaitResult* outResult);
nosResult ResetVBLEvent(void* ctx);

uint64_t NowNs()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct WaitVBLNodeContext : NodeContext
{
	WaitVBLNodeContext() : NodeContext() {}

	nosResult OnCreate(const nosFbNodePtr node) override
	{
		AddPinValueWatcher(NOS_NAME("Channel"), [this](nos::Buffer const& newVal, std::optional<nos::Buffer> oldValue) {
			newVal.As<aja::ChannelInfo>()->UnPackTo(&ChannelInfo);
		});
		AddPinValueWatcher(NOS_NAME("EnableSync"), [this](nos::Buffer const& newVal, std::optional<nos::Buffer> oldValue) {
			if (!oldValue || *oldValue != newVal)
				nosEngine.SendPathRestart(NodeId);
		});
		return NOS_RESULT_SUCCESS;
	}

	struct SyncTimeStartPoint
	{
		uint64_t FrameCount;
		int64_t Clock;
	};
	std::optional<SyncTimeStartPoint> SyncStart;

	nosResult WaitVBL(nosWaitResult* outResult)
	{
		if (auto device = GetDevice())
		{
			auto channel = GetChannel();
			WaitVBL(device.get(),
				channel,
				ChannelInfo.is_input,
				ChannelInfo.is_interlaced,
				sys::vulkan::FieldType::ODD); // GetLastVBLTimestamp only updated on odd field for interlaced.
			auto frameCount = GetVBLCount(*device, channel);
			// Calculate frame timestamp with sync start point + frame count diff * delta time
			auto deltaSecs = GetDeltaSeconds(GetVideoFormat(), ChannelInfo.is_interlaced);
			if (SyncStart)
			{
				uint64_t frameNs = SyncStart->Clock + uint64_t((deltaSecs.x * 1'000'000'000.0 * (frameCount - SyncStart->FrameCount)) / double(deltaSecs.y));
				auto steadyClockNowNs = NowNs();
				outResult->TimeSinceLastEventNs = steadyClockNowNs - frameNs;
				std::chrono::steady_clock::duration startTime =
					std::chrono::duration_cast<std::chrono::steady_clock::duration>(
						std::chrono::nanoseconds(frameNs));
				nosEngine.LogD("%s: %s Time Since Last VBL: %llu (Time: %s)",
					ChannelInfo.is_input ? "In" : "Out",
					ChannelInfo.channel_name.c_str(), outResult->TimeSinceLastEventNs,
					std::format("{:%H:%M:%S}", startTime).c_str());
			}
			outResult->EventCount = frameCount;
			return NOS_RESULT_SUCCESS;
		}
		else
		{
			// Tried to wait when channel not configured. Set pending path restart.
			PendingPathRestart = true;
			return NOS_RESULT_FAILED;
		}
	}

	nosResult ResetVBL()
	{
		if (auto device = GetDevice())
		{
			auto channel = GetChannel();
			for (int i = 0; i < 2; ++i)
				WaitVBL(
					device.get(),
					channel,
					ChannelInfo.is_input,
					ChannelInfo.is_interlaced,
					sys::vulkan::FieldType::ODD); // GetLastVBLTimestamp only updated on odd field for interlaced.
			auto steadyClockNowNs = NowNs();
			SyncTimeStartPoint info{};
			info.FrameCount = GetVBLCount(*device, channel);
			info.Clock = steadyClockNowNs;
			SyncStart = info;
			return NOS_RESULT_SUCCESS;
		}
		else
		{
			// Tried to wait when channel not configured. Set pending path restart.
			PendingPathRestart = true;
			return NOS_RESULT_FAILED;
		}
	}

	bool WaitVBL(AJADevice* device, NTV2Channel channel, bool isInput, bool isInterlaced, sys::vulkan::FieldType waitField)
	{
		if (isInterlaced)
		{
			if (waitField == sys::vulkan::FieldType::UNKNOWN || waitField == sys::vulkan::FieldType::PROGRESSIVE)
				VBLState.InterlacedWaitField =
					VBLState.InterlacedWaitField == sys::vulkan::FieldType::EVEN
						? sys::vulkan::FieldType::ODD
						: sys::vulkan::FieldType::EVEN; // Progressive <-> interlaced, keep track of field type
			else
				VBLState.InterlacedWaitField = waitField; // Use field type from pin
		}
		return device->WaitVBL(
			channel, isInput, isInterlaced ? GetFieldId(VBLState.InterlacedWaitField) : NTV2_FIELD_INVALID);
	}

	std::shared_ptr<AJADevice> GetDevice() const
	{
		if (!ChannelInfo.device)
			return nullptr;
		return AJADevice::GetDeviceBySerialNumber(ChannelInfo.device->serial_number);
	}

	NTV2Channel GetChannel()
	{ 
		if (ChannelInfo.channel_name.empty())
			return NTV2_CHANNEL_INVALID;
		return ParseChannel(ChannelInfo.channel_name);
	}

	NTV2VideoFormat GetVideoFormat() const
	{
		if (ChannelInfo.video_format_idx < 0 || ChannelInfo.video_format_idx >= NTV2_MAX_NUM_VIDEO_FORMATS)
			return NTV2_FORMAT_UNKNOWN;
		return static_cast<NTV2VideoFormat>(ChannelInfo.video_format_idx);
	}

	ULWord GetVBLCount(AJADevice& device, NTV2Channel channel)
	{
		ULWord curVBLCount = 0;
		if (ChannelInfo.is_input)
			device.GetInputVerticalInterruptCount(curVBLCount, channel);
		else
			device.GetOutputVerticalInterruptCount(curVBLCount, channel);
		return curVBLCount;
	}

	nosResult ExecuteNode(NodeExecuteParams const& params) override
	{
		uuid const& outId = params[NOS_NAME_STATIC("VBL")].Id;
		uuid const& outVBLCountId = params[NOS_NAME_STATIC("CurrentVBL")].Id;
		nos::sys::vulkan::FieldType waitField = *params.GetPinData<nos::sys::vulkan::FieldType>(NOS_NAME("WaitField"));
		uuid outFieldPinId = params[NOS_NAME("FieldType")].Id;
		auto device = GetDevice();
		if (!device)
			return NOS_RESULT_FAILED;
		auto channelStr = ChannelInfo.channel_name;
		if (channelStr.empty())
			return NOS_RESULT_FAILED;
		auto channel = ParseChannel(ChannelInfo.channel_name);
		if (PendingPathRestart)
		{
			nosEngine.SendPathRestart(outId);
			PendingPathRestart = false;
			return NOS_RESULT_FAILED;
		}
		auto videoFormat = static_cast<NTV2VideoFormat>(ChannelInfo.video_format_idx);
		bool isInterlaced = !IsProgressivePicture(videoFormat);
		bool vblSuccess = false;
		{
			ScopedProfilerEvent _(ChannelInfo.channel_name + " Wait VBL");
			vblSuccess = WaitVBL(device.get(), channel, ChannelInfo.is_input, isInterlaced, waitField);
#if NOS_AJA_DIAGNOSTICS
			uint64_t timepoint = 0;
			if (ChannelInfo.is_input)
			{
				timepoint = device->GetLastInputVerticalInterruptTimestamp(channel);
			}
			else
			{
				timepoint = device->GetLastOutputVerticalInterruptTimestamp(channel);
			}
			std::chrono::system_clock::duration time = std::chrono::duration_cast<std::chrono::system_clock::duration>(
				std::chrono::nanoseconds(timepoint));
			std::chrono::system_clock::duration startTime =
				std::chrono::duration_cast<std::chrono::system_clock::duration>(
					std::chrono::nanoseconds(VBLState.FirstVBLTimestamp));

			//nosEngine.LogI("%s: %s VBL %lld at %s (Start: %s)",
			//			   ChannelInfo.is_input ? "In " : "Out",
			//			   channelStr.c_str(),
			//			   VBLState.FrameCountSincePathStart,
			//			   std::format("{:%H:%M:%S}", time).c_str(),
			//			   std::format("{:%H:%M:%S}", startTime).c_str());
			VBLState.FrameCountSincePathStart++;
#endif
		}
		nosEngine.SetPinValue(outFieldPinId, nos::Buffer::From(isInterlaced ? VBLState.InterlacedWaitField : sys::vulkan::FieldType::PROGRESSIVE));
		ULWord curVBLCount = GetVBLCount(*device, channel);
		if (!vblSuccess)
		{
			nosEngine.TriggerNodeEvent(NodeId, NSN_VBLFailed);
			return NOS_RESULT_FAILED;
		}

		if (ChannelInfo.is_input && VBLState.FirstVBL)
		{
			uint64_t nanoseconds = device->GetLastInputVerticalInterruptTimestamp(channel);
			nosPathCommand firstVblAfterStart{ .Event = NOS_FIRST_VBL_AFTER_START, .VBLTimestampNs = nanoseconds };
			nosEngine.SendPathCommand(outId, firstVblAfterStart);
		}

		if (VBLState.LastVBLCount)
		{
			int64_t vblDiff = (int64_t)curVBLCount - (int64_t)(VBLState.LastVBLCount + 1);
			if (vblDiff > 0)
			{
				assert(vblDiff <= UINT32_MAX);
				FrameDropped(static_cast<uint32_t>(vblDiff), true);
			} 
			else
			{
				if (VBLState.Dropped)
				{
					if (VBLState.FramesSinceLastDrop++ > 50)
					{
						VBLState.Dropped = false;
						VBLState.FramesSinceLastDrop = 0;
						nosEngine.SendPathRestart(outId);
					}
				}
			}
		}
		VBLState.FirstVBL = false;
		VBLState.LastVBLCount = curVBLCount;
		
		nosEngine.SetPinDirty(outId); // This is unnecessary for now, but when we remove automatically setting outputs dirty on execute, this will be required.
		SetPinValue(outVBLCountId, curVBLCount);
		return NOS_RESULT_SUCCESS;
	}

	static NTV2FieldID GetFieldId(sys::vulkan::FieldType type)
	{
		return type == sys::vulkan::FieldType::EVEN ? NTV2_FIELD1 : 
			(type == sys::vulkan::FieldType::ODD ? NTV2_FIELD0 : NTV2_FIELD_INVALID);
	}
	
	struct {
		bool FirstVBL = true;
		ULWord LastVBLCount = 0;
		bool Dropped = false;
		int FramesSinceLastDrop = 0;
		sys::vulkan::FieldType InterlacedWaitField = sys::vulkan::FieldType::EVEN; // Field flipped first, so start with even
#if NOS_AJA_DIAGNOSTICS
		uint64_t FrameCountSincePathStart = 0;
		uint64_t FirstVBLTimestamp = 0;
#endif
	} VBLState;

	void OnPathStartInitiated() override
	{
		VBLState = {};
		if (auto device = GetDevice())
		{
			auto fmt = GetVideoFormat();
			if (fmt == NTV2_FORMAT_UNKNOWN)
				return;
			auto deltaSecs = GetDeltaSeconds(fmt, ChannelInfo.is_interlaced);
			nosRegisterEventParams params{
				.EventGroupId = IsSyncEnabled() ? NOS_SYNC_DEFAULT_EVENT_GROUP_ID : NOS_SYNC_NO_SYNC_EVENT_GROUP_ID,
				.DeltaSeconds = deltaSecs,
				.UserData = this,
				.ResetFn = ResetVBLEvent,
				.WaitFn = WaitVBLEvent,
				.OutEventId = &WaitId,
			};
			nosSync->RegisterEvent(&params);
		}
	}
	void OnPathStart() override
	{
		if (auto device = GetDevice())
		{
			uint64_t vblTimestampNs = 0, vblCount = 0;
			if (WaitId)
			{
				auto res = nosSync->WaitForConsensus(WaitId, &vblTimestampNs, &vblCount);
				if (res != NOS_RESULT_SUCCESS)
				{
					PendingPathRestart = true;
					return;
				}
			}
			auto channel = GetChannel();
			VBLState.LastVBLCount = vblCount;
#if NOS_AJA_DIAGNOSTICS
			VBLState.FirstVBLTimestamp = vblTimestampNs;
			auto realStartVBL = device->GetLastVBLTimestamp(channel, ChannelInfo.is_input);
			std::chrono::system_clock::duration startTime =
				std::chrono::duration_cast<std::chrono::system_clock::duration>(
					std::chrono::nanoseconds(VBLState.FirstVBLTimestamp));

			std::chrono::system_clock::duration realStartTime =
				std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(realStartVBL));
			nosEngine.LogI("%s: %s VBL %lld started at %s (real %s)",
				ChannelInfo.is_input ? "In " : "Out",
				ChannelInfo.channel_name.c_str(),
				VBLState.FrameCountSincePathStart,
						   std::format("{:%H:%M:%S}", startTime).c_str(),
						   std::format("{:%H:%M:%S}", realStartTime).c_str());
#endif
		}
	}

	void OnPathStop() override
	{
		nosSync->UnregisterEvent(WaitId);
		WaitId = 0;
		SyncStart = std::nullopt;
	}

	void FrameDropped(uint32_t dropCount, bool vblMissed)
	{
		VBLState.Dropped = true;
		VBLState.FramesSinceLastDrop = 0;
		nosEngine.LogW("%s: %s dropped %lld frames (%s missed)", ChannelInfo.is_input ? "In" : "Out", ChannelInfo.channel_name.c_str(), dropCount, vblMissed ? "VBL" : "DMA");
	}

	bool IsSyncEnabled()
	{
		auto buf = GetWatchedPinValue(NOS_NAME("EnableSync"));
		if (!buf.has_value() || buf->Size != sizeof(bool))
			return false;
		return *reinterpret_cast<const bool*>(buf->Data) == true;
	}

	static nosResult GetFunctions(size_t* outCount, nosName* outFunctionNames, nosPfnNodeFunctionExecute* outFunction) 
	{
		*outCount = 1;
		if (!outFunctionNames || !outFunction)
			return NOS_RESULT_SUCCESS;

		outFunctionNames[0] = NOS_NAME("Drop");
		outFunction[0] = [](void* ctx, nosFunctionExecuteParams* params)
		{
			WaitVBLNodeContext* context = reinterpret_cast<WaitVBLNodeContext*>(ctx);
			context->FrameDropped(1, false);
			return NOS_RESULT_SUCCESS;
		};

		return NOS_RESULT_SUCCESS; 
	}

	TChannelInfo ChannelInfo{};
	uint64_t WaitId = 0;
	bool PendingPathRestart = false;
};

nosResult WaitVBLEvent(void* ctx, nosWaitResult* outResult)
{
	return (static_cast<struct WaitVBLNodeContext*>(ctx))->WaitVBL(outResult);
}

nosResult ResetVBLEvent(void* ctx)
{
	return (static_cast<struct WaitVBLNodeContext*>(ctx))->ResetVBL();
}

nosResult RegisterWaitVBLNode(nosNodeFunctions* functions)
{
	NOS_BIND_NODE_CLASS(NOS_NAME_STATIC("nos.aja.WaitVBL"), WaitVBLNodeContext, functions)
	return NOS_RESULT_SUCCESS;
}

}