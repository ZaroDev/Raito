/*
MIT License

Copyright (c) 2023 Víctor Falcón Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include "Core/BasicTypes.h"

#if defined(_WIN64)
	#include <SimpleMath.h>
#else
	#error Math types are only supported on Windows
#endif

namespace Raito
{
	constexpr f32 Pi = 3.14159265359f; /**< Pi constant value */
	constexpr f32 Epsilon = 1e-5f;  /**< Epsilon constant value */

#if defined(_WIN64)
	//! 2D Vector; 32 bit floating point components
	using v2 = DirectX::SimpleMath::Vector2;
	
	//! 2D Vector; 32 bit floating point components aligned on a 16 byte boundary
	using v2a = DirectX::XMFLOAT2A;
	
	//! 3D Vector; 32 bit floating point components
	using v3 = DirectX::SimpleMath::Vector3;
	
	//! 3D Vector; 32 bit floating point components aligned on a 16 byte boundary
	using v3a = DirectX::XMFLOAT3A;
	
	//! 4D Vector; 32 bit floating point components
	using v4 = DirectX::SimpleMath::Vector4;
	
	//! 4D Vector; 32 bit floating point components aligned on a 16 byte boundary
	using v4a = DirectX::XMFLOAT4A;
	
	//! 2D Vector; 32 bit unsigned integer components
	using u32v2 = DirectX::XMUINT2;
	
	//! 3D Vector; 32 bit unsigned integer components
	using u32v3 = DirectX::XMUINT3;
	
	//! 4D Vector; 32 bit unsigned integer components
	using u32v4 = DirectX::XMUINT4;
	
	//! 2D Vector; 32 bit signed integer components
	using s32v2 = DirectX::XMINT2;
	
	//! 3D Vector; 32 bit signed integer components
	using s32v3 = DirectX::XMINT3;
	
	//! 4D Vector; 32 bit signed integer components
	using s32v4 = DirectX::XMINT4;

	//! 3x3 Matrix: 32 bit floating point components
	//! NOTE: DirectXMath doesn't have aligned 3x3 matrices
	using m3x3 = DirectX::XMFLOAT3X3; 
	
	// 4x4 Matrix (assumes right-handed cooordinates)
	using m4x4 = DirectX::SimpleMath::Matrix;
	
	//! 4x4 Matrix: 32 bit floating point components aligned on a 16 byte boundary
	using m4x4a = DirectX::XMFLOAT4X4A;
	
	//! Matrix type: Sixteen 32 bit floating point components aligned on a
	//! 16 byte boundary and mapped to four hardware vector registers
	using Matrix = DirectX::XMMATRIX;
	
	//! Vector intrinsic: Four 32 bit floating point components aligned on a 16 byte
	//! boundary and mapped to hardware vector registers
	using Vector = DirectX::XMVECTOR;

	//! Plane structure
	using Plane = DirectX::SimpleMath::Plane;
	
	//! Quaternion structure
	using Quaternion = DirectX::SimpleMath::Quaternion;
	
	//! Color structure
	using Color = DirectX::SimpleMath::Color;
	
	//! Rectangle structure
	using Rectangle = DirectX::SimpleMath::Rectangle;
	
	//! Ray structure
	using Ray = DirectX::SimpleMath::Ray;
#endif
}