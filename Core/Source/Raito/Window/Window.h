#pragma once

namespace Raito
{
	struct WindowInfo
	{
		std::string Title{};
		u32 Height{};
		u32 Width{};
		bool Fullscreen = false;
		bool VSync = false;
	};

	struct SysWindow
	{
		void* WindowHandle = nullptr;
		void* Window = nullptr;
		
		WindowInfo Info;

		std::string Name{};

		u32 ID{};

		bool IsAlive = false;
	};

	

	namespace Window
	{
		bool Initialize(WindowInfo defaultInfo = {});
		bool Update();
		void Shutdown();

		/// <summary>
		/// Creates a window and returns the id
		/// </summary>
		u32 CreateNewWindow(WindowInfo info);

		/// <summary>
		/// Returns the window with a given ID
		/// </summary>
		SysWindow& GetWindow(u32 id);
	}
	

}
