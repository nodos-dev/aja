#pragma once

#include "AJAClient.h"
#include "glm/glm.hpp"
#include "Ring.h"
#include <nosUtil/Thread.h>

#include <nosVulkanSubsystem/nosVulkanSubsystem.h>

namespace nos
{

using Clock = std::chrono::high_resolution_clock;
using Milli = std::chrono::duration<double, std::milli>;
using Micro = std::chrono::duration<double, std::micro>;

struct CopyThread
{
	nos::Name PinName;
	u32 ConnectedPinCount = 0;
    std::atomic_bool Run = true;
    nos::fb::ShowAs PinKind;

	// Ring
    rc<GPURing> GpuRing;
    rc<CPURing> CpuRing;
	u32 RingSize = 0;
	u32 EffectiveRingSize = 0;
	std::atomic_bool TransferInProgress = false; // TODO: Combine these rings into a double ring structure
	// The ring objects above are overwritten on path restart.
	// TODO: Find out other synchronization issues and fix them all
    std::atomic_uint32_t SpareCount = 0;
    
	std::thread Thread;
    NTV2Channel Channel;
	u32 DropCount = 0;
	std::atomic<u32> FrameIDCounter = 0; 
    struct AJAClient *Client = 0;
    NTV2VideoFormat Format = NTV2_FORMAT_UNKNOWN;
    AJADevice::Mode Mode = AJADevice::SL;
    std::atomic<Colorspace> Colorspace = Colorspace::REC709;
    std::atomic<GammaCurve> GammaCurve = GammaCurve::REC709;
    std::atomic_bool NarrowRange = true;
	std::atomic_bool IsOrphan = false;
    
	rc<CPURing::Resource> SSBO;
	rc<GPURing::Resource> CompressedTex;

	struct
	{
		std::chrono::nanoseconds Time;
		u32 Counter;
	} DebugInfo;

	struct Parameters
	{
		nosTextureFieldType FieldType = NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE;
		u32 FrameNumber;
		Clock::time_point T0;
		Clock::time_point T1;
		rc<GPURing> GR;
		rc<CPURing> CR;
		glm::mat4 Colorspace;
		ShaderType Shader;
		rc<GPURing::Resource> CompressedTex;
		rc<CPURing::Resource> SSBO;
		std::string Name;
		uint32_t Debug = 0;
		nosVec2u DispatchSize;
		std::atomic_bool* TransferInProgress = 0;
		uint64_t SubmissionEventHandle;
		nos::fb::vec2u DeltaSeconds;

		bool Interlaced() const { return !(FieldType == NOS_TEXTURE_FIELD_TYPE_PROGRESSIVE || FieldType == NOS_TEXTURE_FIELD_TYPE_UNKNOWN); }
	};

	struct ConversionThread : ConsumerThread<Parameters>
	{
		ConversionThread(CopyThread* parent) : Parent(parent) {}
		virtual ~ConversionThread();
		std::thread Handle;
		CopyThread* Parent;
	};
	struct InputConversionThread : ConversionThread
	{
		using ConversionThread::ConversionThread;
		void Consume(const Parameters& item) override;
	};
	struct OutputConversionThread : ConversionThread
	{
		using ConversionThread::ConversionThread;
		void Consume(const Parameters& item) override;
	};

	ru<ConversionThread> Worker;
	nosTextureFieldType FieldType = NOS_TEXTURE_FIELD_TYPE_UNKNOWN;

    CopyThread(struct AJAClient *client, u32 ringSize, u32 spareCount, nos::fb::ShowAs kind, NTV2Channel channel, 
                NTV2VideoFormat initalFmt, 
                AJADevice::Mode mode,
                enum class Colorspace colorspace, enum class GammaCurve curve, bool narrowRange, const sys::vulkan::Texture* tex);

    nos::Name const& Name() const;
	nosVec2u GetSuitableDispatchSize() const;
	nosVec2u Extent() const;
    bool IsInput() const;
    void AJAInputProc();
    void AJAOutputProc();
    void CreateRings();
    void SendDeleteRequest();
    void ChangePinResolution(nosVec2u res);
    void InputUpdate(AJADevice::Mode &prevMode);
	bool WaitForVBL(nosTextureFieldType writeField);
	
	void Refresh();
    bool IsQuad() const;
    bool Interlaced() const;
	bool LinkSizeMismatch() const;

    void StartThread();
    void Orphan(bool, std::string const& msg = "");
    void Live(bool);
    void PinUpdate(std::optional<nos::fb::TOrphanState>, Action live);

    void Stop();
	void SetRingSize(u32 ringSize);
	void Restart(u32 ringSize);
    void SetFrame(u32 doubleBufferIndex);
	u32 GetFrameIndex(u32 doubleBufferIndex) const;

	nos::fb::vec2u GetDeltaSeconds() const;

	struct DMAInfo
	{
		u32* Buffer;
		u32 Pitch;
		u32 Segments;
		u32 FrameIndex;
	};

	DMAInfo GetDMAInfo(nosResourceShareInfo& buffer, u32 doubleBufferIndex) const;

    ~CopyThread();

    u32 BitWidth() const;

    void UpdateCurve(enum GammaCurve curve);

    std::array<f64, 2> GetCoeffs() const;

    template<class T>
    glm::mat<4,4,T> GetMatrix() const;

	void NotifyRestart(RestartParams const& params);
	void NotifyDrop();

	u32 TotalFrameCount();

	void SendRingStats();
};

} // namespace nos