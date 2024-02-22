/*
 * Copyright MediaZ AS. All Rights Reserved.
 */

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

constexpr auto AJA_MAX_RING_SIZE = 120u;

struct CopyThread
{
	nos::Name PinName;
	u32 ConnectedPinCount = 0;
    std::atomic_bool Run = true;
    nos::fb::ShowAs PinKind;

	// Ring
    rc<CPURing> Ring;
	u32 RingSize = 0;
	u32 EffectiveRingSize = 0;
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
    std::atomic<aja::Colorspace> Colorspace = aja::Colorspace::REC709;
    std::atomic<aja::GammaCurve> GammaCurve = aja::GammaCurve::REC709;
    std::atomic_bool NarrowRange = true;
	std::atomic_bool IsOrphan = false;
	std::atomic_bool ShouldResetRings = true; // out: fill, input: clear
	bool PendingRestart = false;
    
	rc<CPURing::Resource> SSBO;
	rc<GPURing::Resource> ConversionIntermediateTex;

	nosTextureFieldType OutFieldType = NOS_TEXTURE_FIELD_TYPE_UNKNOWN;

    CopyThread(struct AJAClient *client, u32 ringSize, u32 spareCount, nos::fb::ShowAs kind, NTV2Channel channel, 
                NTV2VideoFormat initalFmt, 
                AJADevice::Mode mode,
                aja::Colorspace colorspace, aja::GammaCurve curve, bool narrowRange, const sys::vulkan::Texture* tex);

    nos::Name const& Name() const;
	nosVec2u GetSuitableDispatchSize() const;
	nosVec2u Extent() const;
    bool IsInput() const;
    void AJAInputProc();
    void AJAOutputProc();
    void CreateRings();
    void SendDeleteRequest();
    void ChangePinResolution(nosVec2u res);
    void InputUpdate(AJADevice::Mode &prevMode, nosTextureFieldType& field);
	bool WaitForVBL(nosTextureFieldType writeField);
	
	void Refresh();
    bool IsQuad() const;
    bool Interlaced() const;
	bool LinkSizeMismatch() const;

    void StartThread();
    void Orphan(bool, std::string const& msg = "");
    void Live(bool);
    void PinUpdate(std::optional<nos::fb::TOrphanState>, Action live);

    bool IsFull();

    void Stop();
	bool SetRingSize(u32 ringSize);
	void Restart(u32 ringSize);
    void SetFrame(u32 doubleBufferIndex);
	u32 GetFrameIndex(u32 doubleBufferIndex) const;

	nosVec2u GetDeltaSeconds() const;

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

    void UpdateCurve(aja::GammaCurve curve);

    std::array<f64, 2> GetCoeffs() const;

    template<class T>
    glm::mat<4,4,T> GetMatrix() const;

	void NotifyRestart(u32 ringSize = 0, nosPathEvent pathEvent = NOS_DROP);

	void SendRingStats();

    void ResetVBLEvent();
};

} // namespace nos