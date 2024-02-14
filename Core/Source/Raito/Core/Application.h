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

#include "Renderer/GraphicsAPI.h"
#include "ECS/Scene.h"

namespace Raito::Core 
{
	//! Application info struct
	//! Defines the initialization values for the application
	struct ApplicationInfo
	{
		std::string Name = "Raito - App"; /**< Name of the application */
		
		u32 Width = 1280; /**< Starting screen width */
		u32 Height = 720; /**< Starting screen height */

		Renderer::API GraphicsAPI = Renderer::API::NONE; /**< Graphics API provider */

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
		virtual ~Application() = default;

		DISABLE_MOVE_AND_COPY(Application)

		//! Initialization function
		/*!
		* Initializes all the modules and calls OnInit at the end if all succeed.
		*/
		bool Initialize();
		//! EndTimeUpdate function
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
		*  Reminder: It always completes the EndTimeUpdate() call before shutting down the application
		*/
		void Close() { m_Running = false; }

	public:
		// Test scene
		ECS::Scene Scene{};

	protected:

		//! Initialization event function
		virtual bool OnInit() { return true; }

		//! ImGui rendering event function
		//! Note: Called after OnUpdate()
		virtual bool OnRenderGUI() { return true; }

		//! EndTimeUpdate event function
		//! Note: Called before OnRenderGUI()
		virtual bool OnUpdate() { return true; }

		//! Shutdown event function
		virtual void OnShutdown() {}

		//! Application constructor
		Application() = default;

		bool m_Running = false; /**< Flag for the application loop to be ran */
	private:
		

		inline static Application* s_Application = nullptr; /**< Static pointer to the instance of the application */
		ApplicationInfo m_Info{}; /**< Info of the current application */

		friend int RunApp(Application& app, ApplicationInfo info, int argc, char** argv);
	};
	//! Application creation function
	//! Initializes and runs the application with the parameters
	int RunApp(Application& app, ApplicationInfo info, int argc, char** argv); 
}

#ifndef DIST
//! Application entry point macro
//! @param app_class Class that inherits from Application to be ran
//! @param app_info Application info
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

