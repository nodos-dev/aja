// Copyright MediaZ AS. All Rights Reserved.


#include "CopyThread.h"
#include "AppEvents_generated.h"
#include "Ring.h"

#include <chrono>

#include <nosVulkanSubsystem/Helpers.hpp>
#include <nosUtil/Stopwatch.hpp>

namespace nos
{

static std::set<u32> const& FindDivisors(const u32 N)
{
	static std::map<u32, std::set<u32>> Map;

	auto it = Map.find(N);
	if(it != Map.end()) 
		return it->second;

	u32 p2 = 0, p3 = 0, p5 = 0;
	std::set<u32> D;
	u32 n = N;
	while(0 == n % 2) n /= 2, p2++;
	while(0 == n % 3) n /= 3, p3++;
	while(0 == n % 5) n /= 5, p5++;
	
	for(u32 i = 0; i <= p2; ++i)
		for(u32 j = 0; j <= p3; ++j)
			for(u32 k = 0; k <= p5; ++k)
				D.insert(pow(2, i) * pow(3, j) * pow(5, k));

	static std::mutex Lock;
	Lock.lock();
	std::set<u32> const& re = (Map[N] = std::move(D));
	Lock.unlock();
	return re;
}

auto LUTFn(bool input, aja::GammaCurve curve) -> f64 (*)(f64)
{
	switch (curve)
	{
	case aja::GammaCurve::REC709:
	default:
		return input ? [](f64 c) -> f64 { return (c < 0.081) ? (c / 4.5) : pow((c + 0.099) / 1.099, 1.0 / 0.45); }
					 : [](f64 c) -> f64 { return (c < 0.018) ? (c * 4.5) : (pow(c, 0.45) * 1.099 - 0.099); };
	case aja::GammaCurve::HLG:
		return input
			   ? [](f64 c)
					 -> f64 { return (c < 0.5) ? (c * c / 3) : (exp(c / 0.17883277 - 5.61582460179) + 0.02372241); }
			   : [](f64 c) -> f64 {
					 return (c < 1. / 12.) ? sqrt(c * 3) : (std::log(c - 0.02372241) * 0.17883277 + 1.00429346);
				 };
	case aja::GammaCurve::ST2084:
		return input ? 
				[](f64 c) -> f64 { c = pow(c, 0.01268331); return pow(glm::max(c - 0.8359375f, 0.) / (18.8515625  - 18.6875 * c), 6.27739463); } : 
				[](f64 c) -> f64 { c = pow(c, 0.15930175); return pow((0.8359375 + 18.8515625 * c) / (1 + 18.6875 * c), 78.84375); };
	}
}

static std::vector<u16> GetGammaLUT(bool input, aja::GammaCurve curve, u16 bits)
{
	std::vector<u16> re(1 << bits, 0.f);
	auto fn = LUTFn(input, curve);
	for (u32 i = 0; i < 1 << bits; ++i)
	{
		re[i] = u16(f64((1 << 16) - 1) * fn(f64(i) / f64((1 << bits) - 1)) + 0.5);
	}
	return re;
}

nos::Name const& CopyThread::Name() const
{
	return PinName;
}

bool CopyThread::IsInput() const
{
	return PinKind == nos::fb::ShowAs::OUTPUT_PIN;
}

bool CopyThread::IsQuad() const
{
	return AJADevice::IsQuad(Mode);
}

bool CopyThread::LinkSizeMismatch() const
{
	auto in0 = Client->Device->GetVPID(Channel);
	const bool SLSignal = CNTV2VPID::VPIDStandardIsSingleLink(in0.GetStandard());
	if (!IsQuad())
	{
		return !SLSignal;
	}
	
	if(Channel & 3)
		return true;

	// Maybe squares
	if (SLSignal)
	{
		auto in1 = Client->Device->GetVPID(NTV2Channel(Channel + 1));
		auto in2 = Client->Device->GetVPID(NTV2Channel(Channel + 2));
		auto in3 = Client->Device->GetVPID(NTV2Channel(Channel + 3));

		auto fmt0 = in0.GetVideoFormat();
		auto fmt1 = in1.GetVideoFormat();
		auto fmt2 = in2.GetVideoFormat();
		auto fmt3 = in3.GetVideoFormat();

		auto std0 = in0.GetStandard();
		auto std1 = in0.GetStandard();
		auto std2 = in0.GetStandard();
		auto std3 = in0.GetStandard();

		return !((fmt0 == fmt1) && (fmt0 == fmt1) &&
				 (fmt2 == fmt3) && (fmt2 == fmt3) &&
				 (fmt0 == fmt2) && (fmt0 == fmt2));
	}

	return false;
}

bool CopyThread::Interlaced() const
{
	return !IsProgressivePicture(Format);
}

void CopyThread::StartThread()
{
	Refresh();
	Ring->Exit = false;
	Run = true;
	std::string threadName("AJA ");
	threadName += IsInput() ? "In" : "Out";
	threadName += ": " + Name().AsString();

	if (!IsInput())
		OutFieldType = Interlaced() ? NOS_TEXTURE_FIELD_TYPE_EVEN : NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;

	Thread = std::thread([this, threadName] {
		flatbuffers::FlatBufferBuilder fbb;
		// TODO: Add nosEngine.SetThreadName call.
		switch (this->PinKind)
		{
		default:
			UNREACHABLE;
		case nos::fb::ShowAs::INPUT_PIN:
			this->AJAOutputProc();
			break;
		case nos::fb::ShowAs::OUTPUT_PIN:
			this->AJAInputProc();
			break;
		}
	});

	flatbuffers::FlatBufferBuilder fbb;
	HandleEvent(CreateAppEvent(fbb, nos::app::CreateSetThreadNameDirect(fbb, (u64)Thread.native_handle(), (threadName + " DMA Thread").c_str())));
}

nosVec2u CopyThread::Extent() const
{
	u32 width, height;
	Client->Device->GetExtent(Format, Mode, width, height);
	return nosVec2u(width, height);
}

void CopyThread::Stop()
{
	Run = false;
	Ring->Stop();
	if (Thread.joinable())
		Thread.join();
	
	nosCmd cmd;
	nosVulkan->Begin("AJA Copy Thread Stop Submit", &cmd);
	nosCmdEndParams endParams{.ForceSubmit = true};
	nosVulkan->End(cmd, &endParams);
	for (auto& res : Ring->Resources)
		if (res->Params.WaitEvent)
			nosVulkan->WaitGpuEvent(&res->Params.WaitEvent, UINT64_MAX);
}

bool CopyThread::SetRingSize(u32 ringSize)
{
	if (!ringSize || ringSize == RingSize || ringSize > AJA_MAX_RING_SIZE)
		return false;

	RingSize = ringSize;

	if (IsInput())
		nosEngine.SetPinValue(Client->GetPinId(nos::Name(Name().AsString() + " Ring Size")),
							 nosBuffer{.Data = &ringSize, .Size = sizeof(u32)});

	return true;
}

void CopyThread::Restart(u32 ringSize)
{
	ShouldResetRings = true; // if ring size did not change, outs will just refill
	if (SetRingSize(ringSize))
	{
		Stop();
		CreateRings();
		StartThread();
	}
	PendingRestart = false;
}

void CopyThread::SetFrame(u32 doubleBufferIndex)
{
	u32 frameIndex = GetFrameIndex(doubleBufferIndex);
	IsInput() ? Client->Device->SetInputFrame(Channel, frameIndex)
			  : Client->Device->SetOutputFrame(Channel, frameIndex);
	if (IsQuad())
		for (u32 i = Channel + 1; i < Channel + 4; ++i)
			IsInput() ? Client->Device->SetInputFrame(NTV2Channel(i), frameIndex)
					  : Client->Device->SetOutputFrame(NTV2Channel(i), frameIndex);
}

u32 CopyThread::GetFrameIndex(u32 doubleBufferIndex) const { return 2 * Channel + doubleBufferIndex; }

nosVec2u CopyThread::GetDeltaSeconds() const
{
	NTV2FrameRate frameRate = GetNTV2FrameRateFromVideoFormat(Format);
	nosVec2u deltaSeconds = { 1,50 };
	switch (frameRate)
	{
	case NTV2_FRAMERATE_6000:	deltaSeconds = { 1, 60 }; break;
	case NTV2_FRAMERATE_5994:	deltaSeconds = { 1001, 60000 }; break;
	case NTV2_FRAMERATE_3000:	deltaSeconds = { 1, 30 }; break;
	case NTV2_FRAMERATE_2997:	deltaSeconds = { 1001, 30000 }; break;
	case NTV2_FRAMERATE_2500:	deltaSeconds = { 1, 25 }; break;
	case NTV2_FRAMERATE_2400:	deltaSeconds = { 1, 24 }; break;
	case NTV2_FRAMERATE_2398:	deltaSeconds = { 1001, 24000 }; break;
	case NTV2_FRAMERATE_5000:	deltaSeconds = { 1, 50 }; break;
	case NTV2_FRAMERATE_4800:	deltaSeconds = { 1, 48 }; break;
	case NTV2_FRAMERATE_4795:	deltaSeconds = { 1001, 48000 }; break;
	case NTV2_FRAMERATE_12000:	deltaSeconds = { 1, 120 }; break;
	case NTV2_FRAMERATE_11988:	deltaSeconds = { 1001, 120000 }; break;
	case NTV2_FRAMERATE_1500:	deltaSeconds = { 1, 15 }; break;
	case NTV2_FRAMERATE_1498:	deltaSeconds = { 1001, 15000 }; break;
	default:					deltaSeconds = { 1, 50 }; break;
	}
	if (Interlaced())
		deltaSeconds.y = deltaSeconds.y * 2;
	return deltaSeconds;
}

#define SSBO_SIZE 10

void CopyThread::UpdateCurve(aja::GammaCurve curve)
{
	GammaCurve = curve;
	auto data = GetGammaLUT(IsInput(), GammaCurve, SSBO_SIZE);
	auto ptr = nosVulkan->Map(&SSBO->Res);
	memcpy(ptr, data.data(), data.size() * sizeof(data[0]));
}

std::array<f64, 2> CopyThread::GetCoeffs() const
{
	switch (Colorspace)
	{
	case aja::Colorspace::REC601:
		return {.299, .114};
	case aja::Colorspace::REC2020:
		return {.2627, .0593};
	case aja::Colorspace::REC709:
	default:
		return {.2126, .0722};
	}
}

template<class T>
glm::mat<4,4,T> CopyThread::GetMatrix() const
{
	// https://registry.khronos.org/DataFormat/specs/1.3/dataformat.1.3.html#MODEL_CONVERSION
	const auto [R, B] = GetCoeffs();
	const T G = T(1) - R - B; // Colorspace

	/*
	* https://registry.khronos.org/DataFormat/specs/1.3/dataformat.1.3.html#QUANTIZATION_NARROW
		Dequantization:
			n = Bit Width {8, 10, 12}
			Although unnoticable, quantization scales differs between bit widths
			This is merely mathematical perfection the error terms is less than 0.001
	*/

	const T QuantizationScalar = T(1 << (BitWidth() - 8)) / T((1 << BitWidth()) - 1);
	const T Y  = NarrowRange ? 219 * QuantizationScalar : 1;
	const T C  = NarrowRange ? 224 * QuantizationScalar : 1;
	const T YT = NarrowRange ? 16 * QuantizationScalar : 0;
	const T CT = 128 * QuantizationScalar;
	const T CB = .5 * C / (B - 1);
	const T CR = .5 * C / (R - 1);

	const auto V0 = glm::vec<3,T>(R, G, B);
	const auto V1 = V0 - glm::vec<3,T>(0, 0, 1);
	const auto V2 = V0 - glm::vec<3,T>(1, 0, 0);

	return glm::transpose(glm::mat<4,4,T>( 
			glm::vec<4,T>(Y  * V0, YT), 
			glm::vec<4,T>(CB * V1, CT), 
			glm::vec<4,T>(CR * V2, CT), 
			glm::vec<4,T>(0, 0, 0,  1)));
}

void CopyThread::Refresh()
{
	Client->Device->CloseChannel(Channel, Client->Input, IsQuad());
	Client->Device->RouteSignal(Channel, Format, Client->Input, Mode, Client->FBFmt());
	Format = IsInput() ? Client->Device->GetInputVideoFormat(Channel) : Format;
	Client->Device->SetRegisterWriteMode(Interlaced() ? NTV2_REGWRITE_SYNCTOFIELD : NTV2_REGWRITE_SYNCTOFRAME, Channel);
	CreateRings();
}

void CopyThread::CreateRings()
{
	EffectiveRingSize = RingSize * (1 + uint32_t(Interlaced()));
	const auto ext = Extent();
	nosVec2u compressedExt((10 == BitWidth()) ? ((ext.x + (48 - ext.x % 48) % 48) / 3) << 1 : ext.x >> 1, ext.y >> u32(Interlaced()));
	Ring = MakeShared<CPURing>(compressedExt, EffectiveRingSize, nosBufferUsage::NOS_BUFFER_USAGE_TRANSFER_SRC);
	nosTextureInfo info = {};
	info.Width  = compressedExt.x;
	info.Height = compressedExt.y;
	info.Format = NOS_FORMAT_R8G8B8A8_UINT;
	info.Usage = NOS_IMAGE_USAGE_TRANSFER_DST;
	ConversionIntermediateTex = MakeShared<GPURing::Resource>(info);
}

void CopyThread::InputUpdate(AJADevice::Mode& prevMode, nosTextureFieldType& field)
{
	auto fmt = Client->Device->GetInputVideoFormat(Channel);
	if (fmt != Format)
	{
		const bool changeRes = GetNTV2FrameGeometryFromVideoFormat(fmt) != GetNTV2FrameGeometryFromVideoFormat(Format);
		Refresh();
		if (changeRes)
			ChangePinResolution(Extent());
		std::string fmtString = NTV2VideoFormatToString(fmt, true);
		std::vector<u8> fmtData(fmtString.data(), fmtString.data() + fmtString.size() + 1);
		nosEngine.SetPinValueByName(Client->Mapping.NodeId,  nos::Name(PinName.AsString() + " Video Format"), nosBuffer{.Data = fmtData.data(), .Size = fmtData.size()});
	}

	if (Interlaced() ^ vkss::IsTextureFieldTypeInterlaced(field))
		field = Interlaced() ? NOS_TEXTURE_FIELD_TYPE_EVEN : NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;

	if (Mode == AJADevice::AUTO)
	{
		auto curMode = Client->Device->GetMode(Channel);
		if (prevMode != curMode)
		{
			prevMode = curMode;
			Refresh();
		}
	}

#pragma push_macro("Q")
#pragma push_macro("R")
#define Q(N)                                                                                                           \
	case N: {                                                                                                          \
		reg = kRegRXSDI##N##FrameCountLow; /* Is only getting the low reg enough? */                                   \
		break;                                                                                                         \
	}
#define R(C)                                                                                                           \
	switch (C + 1)                                                                                                     \
	{                                                                                                                  \
		Q(1) Q(2) Q(3) Q(4) Q(5) Q(6) Q(7) Q(8)                                                                        \
	}
	NTV2RXSDIStatusRegister reg;
	R(Channel);
#pragma pop_macro("Q")
#pragma pop_macro("R")

	u32 val;
	Client->Device->ReadRegister(reg, val);
	FrameIDCounter.store(val);
}



CopyThread::DMAInfo CopyThread::GetDMAInfo(nosResourceShareInfo& buffer, u32 doubleBufferIndex) const
{
	return {
		.Buffer = (u32*)nosVulkan->Map(&buffer),
		.Pitch = ConversionIntermediateTex->Res.Info.Texture.Width * 4,
		.Segments = ConversionIntermediateTex->Res.Info.Texture.Height,
		.FrameIndex = GetFrameIndex(doubleBufferIndex)
	};
}

bool CopyThread::WaitForVBL(nosTextureFieldType writeField)
{
	bool ret;
	if (Interlaced())
	{
		auto waitField = vkss::FlippedField(writeField);
		auto fieldId = GetAJAFieldID(waitField);
		ret = IsInput() ? Client->Device->WaitForInputFieldID(fieldId, Channel)
						: Client->Device->WaitForOutputFieldID(fieldId, Channel);
	}
	else
	{
		ret = IsInput() ? Client->Device->WaitForInputVerticalInterrupt(Channel)
						: Client->Device->WaitForOutputVerticalInterrupt(Channel);
	}

	return ret;
}


void CopyThread::AJAInputProc()
{
	Orphan(false);
	nosEngine.LogI("AJAIn (%s) Thread: %d", Name().AsCStr(), std::this_thread::get_id());

	auto prevMode = Client->Device->GetMode(Channel);

	ResetVBLEvent();
	
	u32 doubleBufferIndex = uint32_t(!Interlaced());
	SetFrame(doubleBufferIndex);
	if (!Interlaced())
	doubleBufferIndex ^= 1;

	auto field = Interlaced() ? NOS_TEXTURE_FIELD_TYPE_EVEN : NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;
	WaitForVBL(field);

	DropCount = 0;
	u64 frameCount = 0;
	ULWord lastVBLCount = 0;
	bool dropped = false;
	uint64_t framesSinceLastDrop = 0;

	auto deltaSec = GetDeltaSeconds();
	uint64_t frameTimeNs = (deltaSec.x / static_cast<double>(deltaSec.y)) * 1'000'000'000;
	ShouldResetRings = true;

	while (Run && !Ring->Exit)
	{
	#pragma region Clear Due To Restart Signal
		if (ShouldResetRings)
		{ 
			nosEngine.LogI("In: %s restarting", Name().AsCStr());
			Ring->Reset(false);
			lastVBLCount = 0;
			ShouldResetRings = false;
			framesSinceLastDrop = 0;
			dropped = 0;
			ResetVBLEvent();
		}
	#pragma endregion

		SendRingStats();

	#pragma region Check Input Signal
		InputUpdate(prevMode, field);

		if (LinkSizeMismatch())
		{
			Orphan(true, "Quad - Single link mismatch");
			do
			{
				if (!Run || Ring->Exit)
				{
					goto EXIT;
				}
				WaitForVBL(field);
				InputUpdate(prevMode, field);
			} while (LinkSizeMismatch());
			Orphan(false);
		}
	#pragma endregion

	#pragma region Wait For VBL & Ring
		if (!WaitForVBL(field))
		{
			field = vkss::FlippedField(field);
			Orphan(true, "AJA Input has no signal");
			while (!WaitForVBL(field))
			{
				field = vkss::FlippedField(field);
				if (!Run || Ring->Exit)
				{
					goto EXIT;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(4));
				InputUpdate(prevMode, field);
			}
			InputUpdate(prevMode, field);
			Orphan(false);
		}
		CPURing::Resource* slot = Ring->BeginPush();
		if (!slot)
		{
			nosEngine.LogW("In: %s push failed", Name().AsCStr());
			continue;
		}

		if (slot->Params.WaitEvent)
		{
			util::Stopwatch swGPU{};
			if (NOS_RESULT_SUCCESS != nosVulkan->WaitGpuEvent(&slot->Params.WaitEvent, frameTimeNs * 10))
			{
				nosEngine.LogW("In: GPU stalled for more than 10 frames, expect tearing.");
			}
			nosEngine.WatchLog("AJA Input GPU Wait Time", swGPU.ElapsedString().c_str());
		}
	#pragma endregion

	#pragma region Drop Calculations
		ULWord curVBLCount;
		Client->Device->GetInputVerticalInterruptCount(curVBLCount, Channel);

		if (lastVBLCount)
		{
			int64_t vblDiff = (int64_t) curVBLCount - (int64_t)(lastVBLCount + 1 + Interlaced());
			if (vblDiff > 0)
			{
				DropCount += vblDiff;
				dropped = true;
				framesSinceLastDrop = 0;
				nosEngine.LogW("In: %s dropped %lld frames", Name().AsCStr(), vblDiff);
			}
			else if (dropped)
			{
				if (framesSinceLastDrop++ > 50)
				{
					dropped = false;
					framesSinceLastDrop = 0;
					NotifyRestart(0, NOS_DROP);
				}
			}
		}
		lastVBLCount = curVBLCount;
	#pragma endregion

	#pragma region DMA
		util::Stopwatch swDma;
		auto [Buf, Pitch, Segments, FrameIndex] = GetDMAInfo(slot->Res, doubleBufferIndex);
		if (Interlaced())
		{
			auto fieldId = (u32(field) - 1);
			Client->Device->DMAReadSegments(FrameIndex,
											Buf,									// target CPU buffer address
											fieldId * Pitch, // source AJA buffer address
											Pitch,									// length of one line
											Segments,								// number of lines
											Pitch,		// increment target buffer one line on CPU memory
											Pitch * 2); // increment AJA card source buffer double the size of one line
														// for ex. next odd line or next even line
		}
		else
			Client->Device->DMAReadFrame(FrameIndex, Buf, Pitch * Segments, Channel);
		nosEngine.WatchLog("AJA Input DMA Time", swDma.ElapsedString().c_str());
	#pragma endregion

	#pragma region Push To Ring
		frameCount++;
		slot->FrameNumber = frameCount;
		slot->Params.FieldType = field;
		slot->Params.ColorspaceMatrix = glm::inverse(GetMatrix<f64>());
		Ring->EndPush(slot);
	#pragma endregion

	#pragma region Update Next Frame Info
		if (!Interlaced())
		{
			SetFrame(doubleBufferIndex);
			doubleBufferIndex ^= 1;
		}
		
		field = vkss::FlippedField(field);
	#pragma endregion	

	}
EXIT:

	Ring->Stop();

	if (Run)
	{
		SendDeleteRequest();
	}
}

nosVec2u CopyThread::GetSuitableDispatchSize() const
{
	constexpr auto BestFit = [](i64 val, i64 res) -> u32 {
		auto d = FindDivisors(res);
		auto it = d.upper_bound(val);
		if (it == d.begin())
			return *it;
		if (it == d.end())
			return res;
		const i64 hi = *it;
		const i64 lo = *--it;
		return u32(abs(val - lo) < abs(val - hi) ? lo : hi);
	};

	const u32 q = IsQuad();
	f32 x = glm::clamp<u32>(Client->DispatchSizeX.load(), 1, ConversionIntermediateTex->Res.Info.Texture.Width) * (1 + q) * (.25 * BitWidth() - 1);
	f32 y = glm::clamp<u32>(Client->DispatchSizeY.load(), 1, ConversionIntermediateTex->Res.Info.Texture.Height) * (1. + q) * (1 + Interlaced());

	return nosVec2u(BestFit(x + .5, ConversionIntermediateTex->Res.Info.Texture.Width >> (BitWidth() - 5)),
					 BestFit(y + .5, ConversionIntermediateTex->Res.Info.Texture.Height / 9));
}

void CopyThread::NotifyRestart(u32 ringSize /* = 0*/, nosPathEvent pathEvent /* = MZ_OUTPUT_DROP*/)
{
	if (PendingRestart && ringSize == 0)
		return;
	nosEngine.LogW("%s is notifying path for restart", Name().AsCStr());
	auto id = Client->GetPinId(Name());
	nosEngine.SendPathCommand(
		nosPathCommand{.Event = ringSize ? NOS_RING_SIZE_CHANGE : pathEvent, .PinId = id, .RingSize = ringSize});
	PendingRestart = true;
}

void CopyThread::AJAOutputProc()
{
	flatbuffers::FlatBufferBuilder fbb;
	auto id = Client->GetPinId(PinName);
	auto deltaSec = GetDeltaSeconds();
	Orphan(false);
	nosEngine.LogI("AJAOut (%s) Thread: %d", Name().AsCStr(), std::this_thread::get_id());

	ResetVBLEvent();

	u32 doubleBufferIndex = uint32_t(!Interlaced());
	SetFrame(doubleBufferIndex);
	if (!Interlaced())
	doubleBufferIndex ^= 1;
	DropCount = 0;
	ULWord lastVBLCount = 0;
	bool dropped = false;
	uint64_t framesSinceLastDrop = 0;
	ShouldResetRings = true;

	while (Run && !Ring->Exit)
	{
		if (ShouldResetRings)
		{
			nosSchedulePinParams scheduleParams{id, 0, true, deltaSec, false};
			nosEngine.SchedulePin(&scheduleParams);
			nosEngine.LogI("Out: %s restarting", Name().AsCStr());
			Ring->Reset(true);
			lastVBLCount = 0;
			ShouldResetRings = false;
			dropped = false;
			framesSinceLastDrop = 0;
		}
		SendRingStats();

	#pragma region Wait For Ring & VBL
		auto *slot = Ring->BeginPop();
		if (!slot)
		{
			nosEngine.LogW("Out: %s pop failed", Name().AsCStr());
			continue;
		}
		const auto field = slot->Params.FieldType;

		if (!WaitForVBL(field) || Ring->Exit)
		{
			if (slot->Params.WaitEvent)
				nosVulkan->WaitGpuEvent(&slot->Params.WaitEvent, UINT64_MAX);
			Ring->EndPop(slot);
			break;
		}
		{
			util::Stopwatch swGPU{};
			if (slot->Params.WaitEvent)
				nosVulkan->WaitGpuEvent(&slot->Params.WaitEvent, UINT64_MAX);
			nosEngine.WatchLog("AJA Output GPU Wait Time", swGPU.ElapsedString().c_str());
		}
	#pragma endregion

	#pragma region Drop Calculations
		ULWord curVBLCount;
		Client->Device->GetOutputVerticalInterruptCount(curVBLCount, Channel);
		// Drop calculations:
		
		if (lastVBLCount && !ShouldResetRings)
		{
			int64_t vblDiff = (int64_t)curVBLCount - (int64_t)(lastVBLCount + 1 + Interlaced());
			if (vblDiff > 0)
			{
				DropCount += vblDiff;
				nosEngine.LogW("Out: %s dropped %lld frames", Name().AsCStr(), vblDiff);
				dropped = true;
				framesSinceLastDrop = 0;
			}
			else if (dropped)
			{
				if (framesSinceLastDrop++ >= 50)
				{
					dropped = false;
					framesSinceLastDrop = 0;
					NotifyRestart(0, NOS_DROP);
				}
			}
		}
		lastVBLCount = curVBLCount;
	#pragma endregion

	#pragma region DMA
		util::Stopwatch swDma;
		auto [Buf, Pitch, Segments, FrameIndex] = GetDMAInfo(slot->Res, doubleBufferIndex);
		if (Interlaced())
		{
			auto fieldId = GetAJAFieldID(field);
			Client->Device->DMAWriteSegments(FrameIndex, Buf, fieldId * Pitch, Pitch, Segments, Pitch, Pitch * 2);
		}
		else
			Client->Device->DMAWriteFrame(FrameIndex, Buf, Pitch * Segments, Channel);
		nosEngine.WatchLog("AJA Output DMA Time", swDma.ElapsedString().c_str());
	#pragma endregion
		
	#pragma region Update Next Frame Info
		if (!Interlaced())
		{
			SetFrame(doubleBufferIndex);
			doubleBufferIndex ^= 1;
		}
	#pragma endregion
	
		Ring->EndPop(slot);

		nosSchedulePinParams scheduleParams{id, 1, false, deltaSec, false};
		nosEngine.SchedulePin(&scheduleParams);
	}
	Ring->Stop();

	nosEngine.EndScheduling(id);

	if (Run)
		SendDeleteRequest();
}

void CopyThread::SendDeleteRequest()
{
	flatbuffers::FlatBufferBuilder fbb;
	auto ids = Client->GeneratePinIDSet(nos::Name(Name()), Mode);
	HandleEvent(
		CreateAppEvent(fbb, nos::CreatePartialNodeUpdateDirect(fbb, &Client->Mapping.NodeId, ClearFlags::NONE, &ids)));
}

void CopyThread::ChangePinResolution(nosVec2u res)
{
	sys::vulkan::TTexture tex;
	tex.width = res.x;
	tex.height = res.y;
	tex.unscaled = true;
	tex.unmanaged = !IsInput();
	tex.format = sys::vulkan::Format::R16G16B16A16_UNORM;
	flatbuffers::FlatBufferBuilder fbb;
	fbb.Finish(sys::vulkan::CreateTexture(fbb, &tex));
	auto val = fbb.Release();
	nosEngine.SetPinValueByName(Client->Mapping.NodeId, PinName, {val.data(), val.size()} );
}

CopyThread::CopyThread(struct AJAClient *client, u32 ringSize, u32 spareCount, nos::fb::ShowAs kind, 
					   NTV2Channel channel, NTV2VideoFormat initalFmt,
					   AJADevice::Mode mode, aja::Colorspace colorspace, aja::GammaCurve curve,
					   bool narrowRange, const sys::vulkan::Texture* tex)
	: PinName(GetChannelStr(channel, mode)), Client(client), PinKind(kind), Channel(channel), SpareCount(spareCount), Mode(mode),
	  Colorspace(colorspace), GammaCurve(curve), NarrowRange(narrowRange), Format(initalFmt)
{
	{
		nosBufferInfo info = {};
		info.Size = (1<<(SSBO_SIZE)) * sizeof(u16);
		info.Usage = NOS_BUFFER_USAGE_STORAGE_BUFFER; // | NOS_BUFFER_USAGE_DEVICE_MEMORY;
		SSBO = MakeShared<CPURing::Resource>(info);
		UpdateCurve(GammaCurve);
	}

	RingSize = ringSize;

	client->Device->SetRegisterWriteMode(Interlaced() ? NTV2_REGWRITE_SYNCTOFIELD : NTV2_REGWRITE_SYNCTOFRAME, Channel);

	CreateRings();
	StartThread();
}

CopyThread::~CopyThread()
{
	Stop();
	Client->Device->CloseChannel(Channel, IsInput(), IsQuad());
}

void CopyThread::Orphan(bool orphan, std::string const& message)
{
	IsOrphan = orphan;
	PinUpdate(nos::fb::TOrphanState{.is_orphan=orphan, .message=message}, Action::NOP);
}

void CopyThread::Live(bool b)
{
	PinUpdate(std::nullopt, b ? Action::SET : Action::RESET);
}

void CopyThread::PinUpdate(std::optional<nos::fb::TOrphanState> orphan, nos::Action live)
{
	flatbuffers::FlatBufferBuilder fbb;
	auto ids = Client->GeneratePinIDSet(nos::Name(Name()), Mode);
	std::vector<flatbuffers::Offset<PartialPinUpdate>> updates;
	std::transform(ids.begin(), ids.end(), std::back_inserter(updates),
				   [&fbb, orphan, live](auto id) { return nos::CreatePartialPinUpdateDirect(fbb, &id, 0, orphan ? nos::fb::CreateOrphanState(fbb, &*orphan) : false, live); });
	HandleEvent(
		CreateAppEvent(fbb, nos::CreatePartialNodeUpdateDirect(fbb, &Client->Mapping.NodeId, ClearFlags::NONE, 0, 0, 0,
															  0, 0, 0, 0, &updates)));
}

bool CopyThread::IsFull()
{
	if (Ring->IsFull())
	{
		ShouldResetRings = false;
		return true;
	}

	return false;
}

u32 CopyThread::BitWidth() const
{
	return Client->BitWidth();
}

void CopyThread::SendRingStats() {
	nosEngine.WatchLog((Name().AsString() + " Ring Read Size").c_str(), std::to_string(Ring->Read.Pool.size()).c_str());
	nosEngine.WatchLog((Name().AsString() + " Ring Write Size").c_str(), std::to_string(Ring->Write.Pool.size()).c_str());
	nosEngine.WatchLog((Name().AsString() + " Total Frame Count").c_str(), std::to_string(Ring->TotalFrameCount()).c_str());
	nosEngine.WatchLog((Name().AsString() + " Drop Count").c_str(), std::to_string(DropCount).c_str());
}

void CopyThread::ResetVBLEvent() 
{
	if (IsInput())
	{
		Client->Device->UnsubscribeInputVerticalEvent(Channel);
		Client->Device->SubscribeInputVerticalEvent(Channel);
	}
	else
	{
		Client->Device->UnsubscribeOutputVerticalEvent(Channel);
		Client->Device->SubscribeOutputVerticalEvent(Channel);
	}
}

} // namespace nos
