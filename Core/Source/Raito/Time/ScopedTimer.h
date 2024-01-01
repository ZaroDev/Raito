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
#include <chrono>


namespace Raito
{
	//! Scoped timer class
	/*
	*	Meant to ouput the time that it took for a function to be ran
	*	It does output the time when destroyed
	*/
	class ScopedTimer
	{
	public:
		//! Constructor
		//! @param name Name of the timer
		ScopedTimer(const char* name);
		//! Destructor
		//! Logs the time that the timer took to destroy
		~ScopedTimer();

	private:
		std::string m_Name{}; /**< Name of the timer */
		std::chrono::time_point<std::chrono::steady_clock> m_Start{} /**< Starting time */, m_End{} /**< Ending time */;
		std::chrono::duration<float> m_Duration{} /**< Lifespan duration in ms */;
	};
}

