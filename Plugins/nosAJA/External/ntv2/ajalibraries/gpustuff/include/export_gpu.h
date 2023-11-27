/* SPDX-License-Identifier: MIT */
/**	@file export_gpu.h
 *	Define system dependent import/export macros and libraries.
 *
 *	Copyright (C) 2013 AJA Video Systems, Inc.  Proprietary and Confidential information.  All rights reserved.
 */

#ifndef GPU_EXPORT_H
#define GPU_EXPORT_H

#if defined(AJA_WINDOWS)
	#if defined(AJA_WINDLL)
		#pragma warning (disable : 4251)
		#if defined(AJA_DLL_BUILD)
			#define GPU_EXPORT __declspec(dllexport)
		#else
			#define GPU_EXPORT __declspec(dllimport)
		#endif
	#else 
		#define GPU_EXPORT
	#endif
#else
		#define GPU_EXPORT
#endif


#endif  // ifndef GPU_EXPORT_H
