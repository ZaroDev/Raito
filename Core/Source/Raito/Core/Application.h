#pragma once
#include <string>
#include "BasicTypes.h"

namespace Raito::Core 
{
	struct ApplicationInfo
	{
		std::string Name{};
		u32 Width{};
		u32 Height{};
	};

	/// <summary>
	/// Base class for any application to inherit from or be directly created
	/// </summary>
	class Application
	{
	public:
		static Application& Get() { return *s_Application; }

		virtual ~Application() {}

		bool Initialize();
		bool Update();
		void Shutdown();

	protected:
		virtual bool OnRenderGUI() { return true; }
		virtual bool OnUpdate() { return true; }

		Application() {}

	protected:
		bool m_Running = false;


	private:
		inline static Application* s_Application = nullptr;
		ApplicationInfo m_Info{};

		friend int RunApp(Application app, ApplicationInfo info);
	};
	
	int RunApp(Application app, ApplicationInfo info);
}
Raito::Core::ApplicationInfo CreateInfo(const char* name = "App", u32 width = 1280, u32 height = 720);

#ifndef DIST
#define CREATE_AND_RUN(app_class, app_info)					\
	int main(int argc, int argv)							\
	{														\
		return Raito::Core::RunApp(app_class(), app_info);	\
	}

#else

#define CREATE_AND_RUN(app_class, app_info)																					\
	INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)							\
	{																														\
		return Raito::Core::RunApp(app_class(), app_info);																	\
	}

#endif

