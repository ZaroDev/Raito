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
#include <string>
#include "BasicTypes.h"

namespace Raito::Core 
{
	//! Application info struct
	struct ApplicationInfo
	{
		std::string Name{}; /**< Name of the application */
		
		u32 Width{}; /**< Starting screen width */
		u32 Height{}; /**< Starting screen height */

		bool Fullscreen = false; /**< Flag for the application starting screen mode */
	};

	//! Application base class
	/*!
	* The base class for an application to inherit from, it handles all the modules execution.
	*/
	class Application
	{
	public:
		//! Application getter
		static Application& Get() { return *s_Application; }

		//! Virtual destructor
		virtual ~Application() {}

		//! Initialization function
		/*!
		* Initializes all the modules and calls OnInit at the end if all succed.
		*/
		bool Initialize();
		//! Update function
		/*!
		* Main update loop, runs every module update function and
		* calls OnUpdate() and OnRenderGUI() with this order.
		*/
		bool Update();
		//! Shutdown function
		/*!
		*	Shutdowns every module and closes the application 
		*/
		void Shutdown();

		//! Close function
		/*
		*  Puts a flag for the application to be call Shutdown() 
		*  Reminder: It always compleates the Update() call before shutting down the application
		*/
		void Close() { m_Running = false; }

	protected:
		//!
		virtual bool OnInit() { return true; }
		virtual bool OnRenderGUI() { return true; }
		virtual bool OnUpdate() { return true; }
		virtual void OnShutdown() {}

		Application() {}

	protected:
		bool m_Running = false;


	private:
		inline static Application* s_Application = nullptr;
		ApplicationInfo m_Info{};

		friend int RunApp(Application& app, ApplicationInfo info, int argc, char** argv);
	};
	int RunApp(Application& app, ApplicationInfo info, int argc, char** argv);
}
Raito::Core::ApplicationInfo CreateInfo(const char* name = "App", u32 width = 1280, u32 height = 720);

#ifndef DIST
#define CREATE_AND_RUN(app_class, app_info)						\
	int main(int argc, char** argv)								\
	{															\
		app_class app{};										\
		return Raito::Core::RunApp(app, app_info, argc, argv);	\
	}

#else

#define CREATE_AND_RUN(app_class, app_info)																					\
	INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)							\
	{																														\
		app_class app{};																									\
		return Raito::Core::RunApp(app, app_info, __argc, __argv);															\
	}

#endif

