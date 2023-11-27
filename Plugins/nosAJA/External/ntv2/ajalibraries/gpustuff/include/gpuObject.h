/* SPDX-License-Identifier: MIT */

#ifndef _GPU_OBJECT
#define _GPU_OBJECT
#include "ajabase/common/types.h"
#include "errorList.h"
#include "export_gpu.h"


typedef struct GpuObjectDescRec {
	bool	_useTexture;
	//these fields are used if the object is a texture 
	bool	 _useRenderToTexture;
	uint32_t _width;                     // Buffer Width
	uint32_t _height;                    // Buffer Height   

	uint32_t _numChannels;             //to be populated by CUDA    
	uint32_t _format;//to be populated by GL/D3D/CUDA
	uint32_t _internalformat;//to be populated by GL
	uint32_t _type;//to be populated by GL
	//these fields are used if the object is a buffer
	uint32_t _size;                      // Specifies the surface size if it's non renderable format			
} GpuObjectDesc;

class IGpuObject
{
public:	
	GPU_EXPORT	IGpuObject();
	GPU_EXPORT	virtual ~IGpuObject(){}	
	GPU_EXPORT	virtual void Init(const GpuObjectDesc &desc) = 0;

	GPU_EXPORT	CErrorList& GetErrorList() const;
	GPU_EXPORT	void Error(const std::string& message) const;
	GPU_EXPORT	uint32_t GetSize() const;	
	GPU_EXPORT	uint32_t GetWidth() const;	
	GPU_EXPORT	uint32_t GetHeight() const;
	GPU_EXPORT	uint32_t GetStride() const;
	GPU_EXPORT	uint32_t GetFormat() const;
	GPU_EXPORT	uint32_t GetInternalFormat() const;
	GPU_EXPORT	uint32_t GetType() const;
	GPU_EXPORT	uint32_t GetNumChannels() const;	
	GPU_EXPORT	bool IsTexture() {return _useTexture;}

protected:	
	mutable CErrorList _errorList;

	bool	_useTexture;
	//these fields are used if the object is a texture 
	bool	 _useRenderToTexture;

	uint32_t _width;                     // Buffer Width
	uint32_t _height;                    // Buffer Height   

	uint32_t _numChannels;             
	uint32_t _format;
	uint32_t _internalformat;
	uint32_t _type;

	uint32_t _size;                      // Specifies the surface size if it's non renderable format

	uint32_t _stride;
};

#endif

