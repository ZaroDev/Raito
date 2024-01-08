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

namespace Raito
{
	//! Universal Unique Identifier class
	//! Creates a randomly generated UUID with a 64 bit length
	class UUID
	{
	public:
		//! Default constructor
		UUID();

		//! Assign constructor
		//! @param uuid The UUID to be assigned
		UUID(u64 uuid);

		//! Copy constructor
		//! @param other UUID to copy from
		UUID(const UUID& other) = default;

		//! UUID Operator
		//! Returns the UUID value as an unsigned 64 bit integer
		operator u64() const { return m_UUID; }
	private:
		u64 m_UUID;
	};

}

namespace std
{
	template <typename T> struct hash;
	//! STD Hash UUID implementation
	template<>
	struct hash<Raito::UUID>
	{
		//! Assign operator
		//! Implementation for the STD library to hash the UUID value
		//! @param uuid UUID to hash
		std::size_t operator()(const Raito::UUID& uuid) const noexcept
		{
			return (u64)uuid;
		}
	};

}