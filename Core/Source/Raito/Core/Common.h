﻿/*
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

#include "BasicTypes.h"
#include "Assert.h"
#include <memory>

#ifndef DEBUG_OP

#ifdef DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x)
#endif

#endif


#ifndef DEFAULT_COPY
#define DEFAULT_COPY(T) \
	T(const T&) = default; \
	T& operator=(const T&) = default;
#endif

#ifndef DEFAULT_MOVE
#define DEFAULT_MOVE(T) \
	T(T&&) noexcept = default; \
	T& operator=(T&&) = default;
#endif


#ifndef DEFAULT_MOVE_AND_COPY
#define DEFAULT_MOVE_AND_COPY(T) \
		DEFAULT_MOVE(T) \
		DEFAULT_COPY(T)
#endif


#ifndef DISABLE_COPY
#define DISABLE_COPY(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete; 
#endif

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T) \
	T(T&&) = delete; \
	T& operator=(T&&) = delete;
#endif

#ifndef DISABLE_MOVE_AND_COPY
#define DISABLE_MOVE_AND_COPY(T) \
	DISABLE_MOVE(T) \
	DISABLE_COPY(T) 
#endif

#define NODISCARD [[nodiscard]]

#define USE_STL_VECTOR 1