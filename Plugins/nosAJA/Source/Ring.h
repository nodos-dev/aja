/*
 * Copyright MediaZ AS. All Rights Reserved.
 */

#pragma once


namespace nos
{

template<typename T>
struct TRing
{
    T Sample;
    
    struct Resource
    {
        nosResourceShareInfo Res;
	    struct {
		    nosTextureFieldType FieldType = NOS_TEXTURE_FIELD_TYPE_UNKNOWN;
			glm::mat4 ColorspaceMatrix = {};
			nosGPUEvent WaitEvent = nullptr;
	    } Params {};

        Resource(T r) : Res{}
        {
            if constexpr (std::is_same_v<T, nosBufferInfo>)
            {
                Res.Info.Type = NOS_RESOURCE_TYPE_BUFFER;
                Res.Info.Buffer = r;
            }
            else
            {
                Res.Info.Type = NOS_RESOURCE_TYPE_TEXTURE;
                Res.Info.Texture = r;
            }
            nosVulkan->CreateResource(&Res);
        }
        
        ~Resource()
        { 
            if (Params.WaitEvent)
				nosVulkan->WaitGpuEvent(&Params.WaitEvent, UINT64_MAX);
            nosVulkan->DestroyResource(&Res);
        }

        void Reset()
		{
			if (Params.WaitEvent)
				nosVulkan->WaitGpuEvent(&Params.WaitEvent, 0);
            Params = {};
			FrameNumber = 0;
        }

        std::atomic_uint64_t FrameNumber;
    };

    void Resize(u32 size)
    {
        Write.Pool = {};
        Read.Pool = {};
        Resources.clear();
        for (u32 i = 0; i < size; ++i)
		{
            auto res = MakeShared<Resource>(Sample);
			Resources.push_back(res);
            Write.Pool.push(res.get());
        }
        Size = size;
    }
    
    TRing(nosVec2u extent, u32 Size) 
        requires(std::is_same_v<T, nosBufferInfo>)
        : Extent(extent), Sample()
    {
        Sample.Size = Extent.x * Extent.y * 4;
        Resize(Size);
    }
    
    TRing(nosVec2u extent, u32 Size, nosFormat format = NOS_FORMAT_R16G16B16A16_UNORM)
        requires(std::is_same_v<T, nosTextureInfo>)
        : Extent(extent), Sample()
    {
        Sample.Width = Extent.x;
        Sample.Height = Extent.y;
        Sample.Format = format;
        Resize(Size);
    }

    struct
    {
        std::queue<Resource *> Pool;
        std::mutex Mutex;
        std::condition_variable CV;
    } Write, Read;

    std::vector<rc<Resource>> Resources;

    u32 Size = 0;
    nosVec2u Extent;
    std::atomic_bool Exit = false;
    std::atomic_bool ResetFrameCount = true;

    ~TRing()
    {
        Stop();
        Resources.clear();
    }

    void Stop()
    {
        {
            std::unique_lock l1(Write.Mutex);
            std::unique_lock l2(Read.Mutex);
            Exit = true;
        }
		Write.CV.notify_all();
		Read.CV.notify_all();
    }

    bool IsFull()
    {
        std::unique_lock lock(Read.Mutex);
		return Read.Pool.size() == Resources.size(); 
    }

    bool CanTakeoGPU()
    {
		std::unique_lock lock(Read.Mutex);
		return !Write.Pool.empty();
	}

    bool IsEmpty()
    {
        std::unique_lock lock(Read.Mutex);
        return Read.Pool.empty();
    }

    u32 ReadyFrames()
    {
        std::unique_lock lock(Read.Mutex);
        return Read.Pool.size();
    }

    u32 TotalFrameCount()
    {
        std::unique_lock lock(Write.Mutex);
        return Size - Write.Pool.size();
    }

    Resource *BeginPush()
    {
        std::unique_lock lock(Write.Mutex);
        while (Write.Pool.empty() && !Exit)
        {
            Write.CV.wait(lock);
        }
        if (Exit)
            return 0;
        Resource *res = Write.Pool.front();
        Write.Pool.pop();
        return res;
    }

    void EndPush(Resource *res)
    {
        {
            std::unique_lock lock(Read.Mutex);
            Read.Pool.push(res);
			assert(Read.Pool.size() <= Resources.size());
        }
        Read.CV.notify_one();
    }

    void CancelPush(Resource* res) { EndPop(res); }
    void CancelPop(Resource* res) { EndPush(res); }

    Resource *BeginPop()
    {
        std::unique_lock lock(Read.Mutex);
        while (Read.Pool.empty() && !Exit)
        {
            Read.CV.wait(lock);
        }
        if (Exit)
            return 0;
        auto res = Read.Pool.front();
        Read.Pool.pop();
        return res;
    }

    void EndPop(Resource *res)
    {
        {
            std::unique_lock lock(Write.Mutex);
            res->FrameNumber = 0;
            Write.Pool.push(res);
			assert(Write.Pool.size() <= Resources.size());
        }
        Write.CV.notify_one();
    }

    bool CanPop(u64& frameNumber, u32 spare = 0)
    {
        std::unique_lock lock(Read.Mutex);
        if (Read.Pool.size() > spare)
        {
            auto newFrameNumber = Read.Pool.front()->FrameNumber.load();
            bool result = ResetFrameCount || !frameNumber || newFrameNumber > frameNumber;
            frameNumber = newFrameNumber;
            ResetFrameCount = false;
            return result;
        }

        return false;
    }

    bool CanPush()
    {
        std::unique_lock lock(Write.Mutex);
        return !Write.Pool.empty();
    }

    Resource *TryPush()
    {
        if (CanPush())
            return BeginPush();
        return 0;
    }

    Resource *TryPush(const std::chrono::milliseconds timeout)
    {
		{
            std::unique_lock lock(Write.Mutex);
		    if (Write.Pool.empty())
                Write.CV.wait_for(lock, timeout, [&]{ return CanPush(); });
		}
		return TryPush();
    }

    Resource *TryPop(u64& frameNumber, u32 spare = 0)
    {
        if (CanPop(frameNumber, spare))
            return BeginPop();
        return 0;
	}

    void Reset(bool fill)
    {
        auto& from = fill ? Write : Read;
		auto& to = fill ? Read : Write;
		std::unique_lock l1(Write.Mutex);
		std::unique_lock l2(Read.Mutex);
		while (!from.Pool.empty())
		{
			auto* slot = from.Pool.front();
			from.Pool.pop();
			slot->Reset();
			to.Pool.push(slot);
		}
    }
};

typedef TRing<nosBufferInfo> CPURing;
typedef TRing<nosTextureInfo> GPURing;

} // namespace nos