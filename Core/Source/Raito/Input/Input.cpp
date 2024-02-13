#include <pch.h>
#include "Input.h"

#include <Window/Window.h>

#include <GLFW/glfw3.h>

namespace Raito::Input
{
	bool Input::IsKeyDown(KeyCode keycode)
	{
		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Window::GetWindow(0).Window);
		int state = glfwGetKey(windowHandle, (int)keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Window::GetWindow(0).Window);
		int state = glfwGetMouseButton(windowHandle, (int)button);
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Window::GetWindow(0).Window);

		double x, y;
		glfwGetCursorPos(windowHandle, &x, &y);
		return { (float)x, (float)y };
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* windowHandle = static_cast<GLFWwindow*>(Window::GetWindow(0).Window);
		glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}
}