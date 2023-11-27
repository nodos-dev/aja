/* SPDX-License-Identifier: MIT */

#ifndef _CPU_OBJECT
#define _CPU_OBJECT
#include "ajabase/common/types.h"
#include "errorList.h"
#include "export_gpu.h"


typedef struct CpuObjectDescRec {

	bool	_useTexture;
	//these are required if this object mirrors a texture on the GPU
	uint32_t _width;                     // Buffer Width
	uint32_t _height;                    // Buffer Height   	
	uint32_t _stride;
	uint32_t _strideAlignment;
	uint32_t _numChannels;             //to be populated by CUDA    
	uint32_t _format;//to be populated by GL/D3D/CUDA
	uint32_t _type;//to be populated by GL
	//these fields are used if the object mirrors a buffer
	uint32_t _size;                      // Specifies the surface size if it's non renderable format
	//this field is always used
	uint32_t _addressAlignment;	
} CpuObjectDesc;

class CCpuObject
{
public:	
	GPU_EXPORT	CCpuObject();
	GPU_EXPORT	virtual ~CCpuObject();	
	GPU_EXPORT	void Init(const CpuObjectDesc &desc);	
	GPU_EXPORT	CErrorList& GetErrorList() const;
	GPU_EXPORT	void Error(const std::string& message) const;
	GPU_EXPORT	uint32_t GetSize() const;	
	GPU_EXPORT	uint32_t GetWidth() const;	
	GPU_EXPORT	uint32_t GetHeight() const;
	GPU_EXPORT	uint32_t GetStride() const;
	GPU_EXPORT	uint32_t GetFormat() const;
	GPU_EXPORT	uint32_t GetType() const;
	GPU_EXPORT	uint32_t GetNumChannels() const;	
	GPU_EXPORT	uint8_t* GetVideoBuffer() const;
	GPU_EXPORT	bool IsTexture() {return _useTexture;}

protected:
	void destroy();
	mutable CErrorList _errorList;

	bool	_useTexture;

	uint32_t _width;                     // Buffer Width
	uint32_t _height;                    // Buffer Height   		    
	uint32_t _stride;
	uint32_t _numChannels;             
	uint32_t _format;
	uint32_t _type;

	uint32_t _size;                      // Specifies the surface size if it's non renderable format	
	//these memory buffers are required for the p2h2p transfers
	uint8_t*			videoBufferUnaligned;
	uint8_t*			videoBuffer;
};

#endif

