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

#include "D3D12Common.h"

namespace Raito::Renderer::D3D12::Core
{
    //! D3D12 Renderer initialize function
    //! Creates a device and initializes main rendering systems
    //! @return If success
	bool Initialize();

    //! Shutdown function for the D3D12 renderer
    //! Destroys everything that the renderer handles
	void Shutdown();

    //! Deferred release helper function
    //! Releases the given object
    void DeferredRelease(IUnknown* resource);

    //! Getter for the current frame index of the renderer
    //! @returns The frame index from 0 .. c_FrameBufferCount
	u32 CurrentFrameIndex();

    //! Sets all the flags for the deferred release to 1
    void SetDeferredReleasesFlags();

	//! Generic release function for D3D12 Objects
    //! Releases and nulls the given object
    //! @param resource Resource to release
    template<typename T>
    constexpr  void Release(T*& resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    //! Generic deferred release function for D3D12 Objects
    //! Releases and nulls the given object
    //! @param resource Resource to release
    template<typename T>
    constexpr void DeferredRelease(T*& resource)
    {
	    if(resource)
	    {
            DeferredRelease(resource);
            resource = nullptr;
	    }
    }

    nvrhi::RefCountPtr<D3D12Device> Device();
    nvrhi::DeviceHandle NVDevice();

    //! Surface creation function
    //! @param window window that the surface will be displayed on
    //! @return the created surface
    Surface CreateSurface(SysWindow* window);

    //! Surface remove function
    //! removes and destroys the indicated surface
    //! @param id id of the surface to be removed
    void RemoveSurface(u32 id);


    void ResizeSurface(u32 id, u32 width, u32 height);
    u32 SurfaceWidth(u32 id);
    u32 SurfaceHeight(u32 id);
    u32 GetColorGetAttachment(u32 target, u32 id);
    u32 GetDepthAttachment(u32 id);
    void RenderSurface(u32 id);
}
