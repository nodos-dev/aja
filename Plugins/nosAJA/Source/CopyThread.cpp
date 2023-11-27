
#include "CopyThread.h"
#include "AppEvents_generated.h"
#include "Ring.h"

#include <chrono>

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

auto LUTFn(bool input, GammaCurve curve) -> f64 (*)(f64)
{
	switch (curve)
	{
	case GammaCurve::REC709:
	default:
		return input ? [](f64 c) -> f64 { return (c < 0.081) ? (c / 4.5) : pow((c + 0.099) / 1.099, 1.0 / 0.45); }
					 : [](f64 c) -> f64 { return (c < 0.018) ? (c * 4.5) : (pow(c, 0.45) * 1.099 - 0.099); };
	case GammaCurve::HLG:
		return input
			   ? [](f64 c)
					 -> f64 { return (c < 0.5) ? (c * c / 3) : (exp(c / 0.17883277 - 5.61582460179) + 0.02372241); }
			   : [](f64 c) -> f64 {
					 return (c < 1. / 12.) ? sqrt(c * 3) : (std::log(c - 0.02372241) * 0.17883277 + 1.00429346);
				 };
	case GammaCurve::ST2084:
		return input ? 
				[](f64 c) -> f64 { c = pow(c, 0.01268331); return pow(glm::max(c - 0.8359375f, 0.) / (18.8515625  - 18.6875 * c), 6.27739463); } : 
				[](f64 c) -> f64 { c = pow(c, 0.15930175); return pow((0.8359375 + 18.8515625 * c) / (1 + 18.6875 * c), 78.84375); };
	}
}

static std::vector<u16> GetGammaLUT(bool input, GammaCurve curve, u16 bits)
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
	if (IsInput())
		Worker = std::make_unique<InputConversionThread>(this);
	else
		Worker = std::make_unique<OutputConversionThread>(this);
	CpuRing->Exit = false;
	GpuRing->Exit = false;
	Run = true;
	std::string threadName("AJA ");
	threadName += IsInput() ? "In" : "Out";
	threadName += ": " + Name().AsString();

	Thread = std::thread([this, threadName] {
		Worker->Start();
		flatbuffers::FlatBufferBuilder fbb;
		// TODO: Add nosEngine.SetThreadName call.
		HandleEvent(CreateAppEvent(fbb, nos::app::CreateSetThreadNameDirect(fbb, (u64)Worker->GetNativeHandle(), (threadName + " Conversion Thread").c_str())));
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
		Worker->Stop();
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
	GpuRing->Stop();
	CpuRing->Stop();
	if (Thread.joinable())
		Thread.join();
}

void CopyThread::SetRingSize(u32 ringSize)
{
	RingSize = ringSize;
	EffectiveRingSize = ringSize * (1 + uint32_t(Interlaced()));
}

void CopyThread::Restart(u32 ringSize)
{
	assert(ringSize && ringSize < 200);
	SetRingSize(ringSize);
	Stop();
	CreateRings();
	StartThread();
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

nos::fb::vec2u CopyThread::GetDeltaSeconds() const
{
	NTV2FrameRate frameRate = GetNTV2FrameRateFromVideoFormat(Format);
	nos::fb::vec2u deltaSeconds = { 1,50 };
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
		deltaSeconds.mutate_y(deltaSeconds.y() * 2);
	return deltaSeconds;
}

#define SSBO_SIZE 10

void CopyThread::UpdateCurve(enum GammaCurve curve)
{
	GammaCurve = curve;
	auto data = GetGammaLUT(IsInput(), GammaCurve, SSBO_SIZE);
	auto ptr = nosEngine.Map(&SSBO->Res);
	memcpy(ptr, data.data(), data.size() * sizeof(data[0]));
}

std::array<f64, 2> CopyThread::GetCoeffs() const
{
	switch (Colorspace)
	{
	case Colorspace::REC601:
		return {.299, .114};
	case Colorspace::REC2020:
		return {.2627, .0593};
	case Colorspace::REC709:
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
	const auto ext = Extent();
	GpuRing = MakeShared<GPURing>(ext, EffectiveRingSize);
	nosVec2u compressedExt((10 == BitWidth()) ? ((ext.x + (48 - ext.x % 48) % 48) / 3) << 1 : ext.x >> 1, ext.y >> u32(Interlaced()));
	CpuRing = MakeShared<CPURing>(compressedExt, EffectiveRingSize);
	nosTextureInfo info = {};
	info.Width  = compressedExt.x;
	info.Height = compressedExt.y;
	info.Format = NOS_FORMAT_R8G8B8A8_UINT;
	CompressedTex = MakeShared<GPURing::Resource>(info);
}

void CopyThread::InputUpdate(AJADevice::Mode &prevMode)
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

	if (Interlaced() ^ IsTextureFieldTypeInterlaced(FieldType))
		FieldType = Interlaced() ? NOS_TEXTURE_FIELD_TYPE_EVEN : NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;

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
		.Buffer = (u32*)nosEngine.Map(&buffer),
		.Pitch = CompressedTex->Res.Info.Texture.Width * 4,
		.Segments = CompressedTex->Res.Info.Texture.Height,
		.FrameIndex = GetFrameIndex(doubleBufferIndex)
	};
}

bool CopyThread::WaitForVBL(nosTextureFieldType writeField)
{
	bool ret;
	if (Interlaced())
	{
		auto waitField = Flipped(writeField);
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
	NotifyDrop();
	Orphan(false);
	nosEngine.LogI("AJAIn (%s) Thread: %d", Name().AsCStr(), std::this_thread::get_id());

	auto prevMode = Client->Device->GetMode(Channel);

	// Reset interrupt event status
	Client->Device->UnsubscribeInputVerticalEvent(Channel);
	Client->Device->SubscribeInputVerticalEvent(Channel);

	u32 doubleBufferIndex = 0;
	SetFrame(doubleBufferIndex);

	FieldType = Interlaced() ? NOS_TEXTURE_FIELD_TYPE_EVEN : NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;

	Parameters params = {};

	WaitForVBL(FieldType);

	DebugInfo.Time = std::chrono::nanoseconds(0);
	DebugInfo.Counter = 0;

	DropCount = 0;
	u32 framesSinceLastDrop = 0;

	u64 frameCount = 0;

	while (Run && !GpuRing->Exit)
	{
		SendRingStats();

		InputUpdate(prevMode);

		if (LinkSizeMismatch())
		{
			Orphan(true, "Quad - Single link mismatch");
			do
			{
				if (!Run || GpuRing->Exit || CpuRing->Exit)
				{
					goto EXIT;
				}
				WaitForVBL(FieldType);
				InputUpdate(prevMode);
			} while (LinkSizeMismatch());
			Orphan(false);
		}

		if (!WaitForVBL(FieldType))
		{
			FieldType = Flipped(FieldType);
			Orphan(true, "AJA Input has no signal");
			while (!WaitForVBL(FieldType))
			{
				FieldType = Flipped(FieldType);
				if (!Run || GpuRing->Exit || CpuRing->Exit)
				{
					goto EXIT;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(4));
				InputUpdate(prevMode);
			}
			InputUpdate(prevMode);
			Orphan(false);
		}

		params.FrameNumber = frameCount++;

		int ringSize = EffectiveRingSize;
		int totalFrameCount = TotalFrameCount();

		CPURing::Resource* targetCpuRingSlot = nullptr;
		if (totalFrameCount >= ringSize || (!(targetCpuRingSlot = CpuRing->TryPush())))
		{
			DropCount++;
			GpuRing->ResetFrameCount = true;
			framesSinceLastDrop = 0;
			continue;
		}

		auto [Buf, Pitch, Segments, FrameIndex] = GetDMAInfo(targetCpuRingSlot->Res, doubleBufferIndex);
		params.T0 = Clock::now();
		if (Interlaced())
		{
			auto fieldId = (u32(FieldType) - 1);
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

		CpuRing->EndPush(targetCpuRingSlot);
		++framesSinceLastDrop;
		if (DropCount && framesSinceLastDrop == 50)
			NotifyDrop();

		if (!Interlaced())
		{
			SetFrame(doubleBufferIndex);
			doubleBufferIndex ^= 1;
		}

		params.FieldType = FieldType;
		params.T1 = Clock::now();
		params.GR = GpuRing;
		params.CR = CpuRing;
		params.Colorspace = glm::inverse(GetMatrix<f64>());
		params.Shader = Client->Shader;
		params.CompressedTex = CompressedTex;
		params.SSBO = SSBO;
		params.Name = Name();
		params.Debug = Client->Debug;
		params.DispatchSize = GetSuitableDispatchSize();
		params.TransferInProgress = & TransferInProgress;
		Worker->Enqueue(params);
		FieldType = Flipped(FieldType);
	}
EXIT:

	CpuRing->Stop();
	GpuRing->Stop();

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
	f32 x = glm::clamp<u32>(Client->DispatchSizeX.load(), 1, CompressedTex->Res.Info.Texture.Width) * (1 + q) * (.25 * BitWidth() - 1);
	f32 y = glm::clamp<u32>(Client->DispatchSizeY.load(), 1, CompressedTex->Res.Info.Texture.Height) * (1. + q) * (1 + Interlaced());

	return nosVec2u(BestFit(x + .5, CompressedTex->Res.Info.Texture.Width >> (BitWidth() - 5)),
					 BestFit(y + .5, CompressedTex->Res.Info.Texture.Height / 9));
}

void CopyThread::NotifyRestart(RestartParams const& params)
{
	nosEngine.LogW("%s is notifying path for restart", Name().AsCStr());
	auto id = Client->GetPinId(Name());
	auto args = Buffer::From(params);
	nosEngine.SendPathCommand(nosPathCommand{
		.PinId = id,
		.Command = NOS_PATH_COMMAND_TYPE_RESTART,
		.Execution = NOS_PATH_COMMAND_EXECUTION_TYPE_WALKBACK,
		.Args = nosBuffer { args.data(), args.size() }
	});
}

void CopyThread::NotifyDrop()
{
	if (!ConnectedPinCount)
		return;
	nosEngine.LogW("%s is notifying path about a drop event", Name().AsCStr());
	auto id = Client->GetPinId(nos::Name(Name()));
	auto args = Buffer::From(GpuRing->Size);
	nosEngine.SendPathCommand(nosPathCommand{
		.PinId = id,
		.Command = NOS_PATH_COMMAND_TYPE_NOTIFY_DROP,
		.Execution = NOS_PATH_COMMAND_EXECUTION_TYPE_NOTIFY_ALL_CONNECTIONS,
		.Args = nosBuffer { args.data(), args.size() }
	});
}

u32 CopyThread::TotalFrameCount()
{
	assert(CpuRing->TotalFrameCount() + GpuRing->TotalFrameCount() != 0 || !TransferInProgress);
	return std::max(0, int(CpuRing->TotalFrameCount() + GpuRing->TotalFrameCount()) - (TransferInProgress ? 1 : 0));
}

void CopyThread::AJAOutputProc()
{
	flatbuffers::FlatBufferBuilder fbb;
	auto id = Client->GetPinId(PinName);
	auto deltaSec = GetDeltaSeconds();
	auto hungerSignal = CreateAppEvent(fbb, nos::app::CreateScheduleRequest(fbb, nos::app::ScheduleRequestKind::PIN, &id, false, &deltaSec));
	HandleEvent(hungerSignal);
	Orphan(false);
	nosEngine.LogI("AJAOut (%s) Thread: %d", Name().AsCStr(), std::this_thread::get_id());

	while (Run && !CpuRing->Exit && TotalFrameCount() < EffectiveRingSize)
		std::this_thread::yield();

	//Reset interrupt event status
	Client->Device->UnsubscribeOutputVerticalEvent(Channel);
	Client->Device->SubscribeOutputVerticalEvent(Channel);

	u32 doubleBufferIndex = 0;
	SetFrame(doubleBufferIndex);

	DropCount = 0;
	u32 framesSinceLastDrop = 0;

	auto field = Interlaced() ? NOS_TEXTURE_FIELD_TYPE_EVEN : NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;

	while (Run && !CpuRing->Exit)
	{
		SendRingStats();

		ULWord lastVBLCount;
		Client->Device->GetOutputVerticalInterruptCount(lastVBLCount, Channel);

		auto *sourceCpuRingSlot = CpuRing->BeginPop();
		if (!sourceCpuRingSlot)
			continue;

		auto field = sourceCpuRingSlot->Res.Info.Texture.FieldType;

		if (!WaitForVBL(field))
			break;

		ULWord vblCount;
		Client->Device->GetOutputVerticalInterruptCount(vblCount, Channel);
		
		// Drop calculations:
		if (vblCount > lastVBLCount + 1 + Interlaced())
		{
			DropCount += vblCount - lastVBLCount - 1 - Interlaced();
			framesSinceLastDrop = 0;
			nosEngine.LogW("AJA Out: %s dropped while waiting for a frame", Name().AsCStr());
		}
		else
		{
			++framesSinceLastDrop;
			if (DropCount && framesSinceLastDrop == 50)
			{
				nosEngine.LogE("AJA Out: %s dropped frames, notifying restart", Name().AsCStr());
				NotifyRestart({});
			}
		}

		auto [Buf, Pitch, Segments, FrameIndex] = GetDMAInfo(sourceCpuRingSlot->Res, doubleBufferIndex);
		if (Interlaced())
		{
			auto fieldId = GetAJAFieldID(field);
			Client->Device->DMAWriteSegments(FrameIndex, Buf, fieldId * Pitch, Pitch, Segments, Pitch, Pitch * 2);
		}
		else
			Client->Device->DMAWriteFrame(FrameIndex, Buf, Pitch * Segments, Channel);
		
		if (!Interlaced())
		{
			SetFrame(doubleBufferIndex);
			doubleBufferIndex ^= 1;
		}
		CpuRing->EndPop(sourceCpuRingSlot);
		HandleEvent(hungerSignal);
	}
	GpuRing->Stop();
	CpuRing->Stop();

	HandleEvent(CreateAppEvent(fbb, 
		nos::app::CreateScheduleRequest(fbb, nos::app::ScheduleRequestKind::PIN, &id, true)));

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
	fb::TTexture tex;
	tex.width = res.x;
	tex.height = res.y;
	tex.unscaled = true;
	tex.unmanaged = !IsInput();
	tex.format = nos::fb::Format::R16G16B16A16_UNORM;
	flatbuffers::FlatBufferBuilder fbb;
	fbb.Finish(fb::CreateTexture(fbb, &tex));
	auto val = fbb.Release();
	nosEngine.SetPinValueByName(Client->Mapping.NodeId, PinName, {val.data(), val.size()} );
}

void CopyThread::InputConversionThread::Consume(CopyThread::Parameters const& params)
{
	auto* sourceCPUSlot = params.CR->BeginPop();
	if (!sourceCPUSlot)
		return;
	*params.TransferInProgress = true;
	auto* destGPURes = params.GR->BeginPush();
	if (!destGPURes)
	{
		params.CR->EndPop(sourceCPUSlot);
		*params.TransferInProgress = false;
		return;
	}

	std::vector<nosShaderBinding> inputs;

	uint32_t iflags = (params.Interlaced() ? u32(params.FieldType) : 0) | (params.Shader == ShaderType::Comp10) << 2;

	inputs.emplace_back(ShaderBinding(NSN_Colorspace, params.Colorspace));
	inputs.emplace_back(ShaderBinding(NSN_Source, params.CompressedTex->Res));
	inputs.emplace_back(ShaderBinding(NSN_Interlaced, iflags));
	inputs.emplace_back(ShaderBinding(NSN_ssbo, params.SSBO->Res));

	auto MsgKey = "Input " + params.Name + " DMA";

	nosCmd cmd;
	nosEngine.Begin(&cmd);

	nosEngine.Copy(cmd, &sourceCPUSlot->Res, &params.CompressedTex->Res, params.Debug ? ("(GPUTransfer)" + MsgKey + ":" + std::to_string(params.Debug)).c_str() : 0);
	if (params.Shader != ShaderType::Frag8)
	{
		inputs.emplace_back(ShaderBinding(NSN_Output, destGPURes->Res));
		nosRunComputePassParams pass = {};
		pass.Key = NSN_AJA_YCbCr2RGB_Compute_Pass;
		pass.DispatchSize = params.DispatchSize;
		pass.Bindings = inputs.data();
		pass.BindingCount = inputs.size();
		pass.Benchmark = params.Debug;
		nosEngine.RunComputePass(cmd, &pass);
	}
	else
	{
		nosRunPassParams pass = {};
		pass.Key = NSN_AJA_YCbCr2RGB_Pass;
		pass.Output = destGPURes->Res;
		pass.Bindings = inputs.data();
		pass.BindingCount = inputs.size();
		pass.Benchmark = params.Debug;
		nosEngine.RunPass(cmd, &pass);
	}
	nosEngine.End(cmd);

	destGPURes->FrameNumber = params.FrameNumber;
	destGPURes->Res.Info.Texture.FieldType = (nosTextureFieldType)params.FieldType;
	params.GR->EndPush(destGPURes);
	*params.TransferInProgress = false;
	params.CR->EndPop(sourceCPUSlot);
}

CopyThread::ConversionThread::~ConversionThread()
{
	Stop();
}

void CopyThread::OutputConversionThread::Consume(const Parameters& params)
{
	auto incoming = params.GR->BeginPop();
	if (!incoming)
		return;

	*params.TransferInProgress = true;

	auto outgoing = params.CR->BeginPush();
	if (!outgoing)
	{
		params.GR->EndPop(incoming);
		*params.TransferInProgress = false;
		return;
	}

	auto wantedField = params.FieldType;
	bool outInterlaced = IsTextureFieldTypeInterlaced(wantedField);
	auto incomingField = incoming->Res.Info.Texture.FieldType;
	bool inInterlaced = IsTextureFieldTypeInterlaced(incomingField);
	if ((inInterlaced && outInterlaced) && incomingField != wantedField)
		nosEngine.LogE("%s field mismatch!", Parent->Name().AsCStr());
	outgoing->Res.Info.Texture.FieldType = wantedField;
	
	// 0th bit: is out even
	// 1st bit: is out odd
	// 2nd bit: is input even
	// 3rd bit: is input odd
	// 4th bit: comp10
	uint32_t iflags = ((params.Shader == ShaderType::Comp10) << 4);
	if (outInterlaced)
		iflags |= u32(wantedField);
	if (inInterlaced)
		iflags |= (u32(incomingField) << 2);

	std::vector<nosShaderBinding> inputs;
	inputs.emplace_back(ShaderBinding(NSN_Colorspace, params.Colorspace));
	inputs.emplace_back(ShaderBinding(NSN_Source, incoming->Res));
	inputs.emplace_back(ShaderBinding(NSN_Interlaced, iflags));
	inputs.emplace_back(ShaderBinding(NSN_ssbo, params.SSBO->Res));

	nosCmd cmd;
	nosEngine.Begin(&cmd);

	// watch out for th members, they are not synced
	if (params.Shader != ShaderType::Frag8)
	{
		inputs.emplace_back(ShaderBinding(NSN_Output, params.CompressedTex->Res));
		nosRunComputePassParams pass = {};
		pass.Key = NSN_AJA_RGB2YCbCr_Compute_Pass;
		pass.DispatchSize = params.DispatchSize;
		pass.Bindings = inputs.data();
		pass.BindingCount = inputs.size();
		pass.Benchmark = params.Debug;
		nosEngine.RunComputePass(cmd, &pass);
	}
	else
	{
		nosRunPassParams pass = {};
		pass.Key = NSN_AJA_RGB2YCbCr_Pass;
		pass.Output = params.CompressedTex->Res;
		pass.Bindings = inputs.data();
		pass.BindingCount = inputs.size();
		pass.Benchmark = params.Debug;
		nosEngine.RunPass(cmd, &pass);
	}

	nosEngine.Copy(cmd, &params.CompressedTex->Res, &outgoing->Res, 0);
	nosEngine.WaitEvent(params.SubmissionEventHandle);
	nosEngine.End(cmd);
	params.GR->EndPop(incoming);
	*params.TransferInProgress = false;
	params.CR->EndPush(outgoing);
}

CopyThread::CopyThread(struct AJAClient *client, u32 ringSize, u32 spareCount, nos::fb::ShowAs kind, 
					   NTV2Channel channel, NTV2VideoFormat initalFmt,
					   AJADevice::Mode mode, enum class Colorspace colorspace, enum class GammaCurve curve,
					   bool narrowRange, const fb::Texture* tex)
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

	SetRingSize(ringSize);

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

u32 CopyThread::BitWidth() const
{
	return Client->BitWidth();
}

void CopyThread::SendRingStats() {
	auto cpuRingReadSize = CpuRing->Read.Pool.size();
	auto gpuRingWriteSize = GpuRing->Write.Pool.size();
	auto cpuRingWriteSize = CpuRing->Write.Pool.size();
	auto gpuRingReadSize = GpuRing->Read.Pool.size();
	nosEngine.WatchLog((Name().AsString() + " CPU Ring Read Size").c_str(), std::to_string(cpuRingReadSize).c_str());
	nosEngine.WatchLog((Name().AsString() + " CPU Ring Write Size").c_str(), std::to_string(cpuRingWriteSize).c_str());
	nosEngine.WatchLog((Name().AsString() + " GPU Ring Read Size").c_str(), std::to_string(gpuRingReadSize).c_str());
	nosEngine.WatchLog((Name().AsString() + " GPU Ring Write Size").c_str(), std::to_string(gpuRingWriteSize).c_str());
	nosEngine.WatchLog((Name().AsString() + " Total Frame Count").c_str(), std::to_string(TotalFrameCount()).c_str());
}

} // namespace nos
