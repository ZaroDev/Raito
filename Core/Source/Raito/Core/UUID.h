#pragma once

namespace Raito {

	class UUID
	{
	public:
		UUID();
		UUID(u64 uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Raito::UUID>
	{
		std::size_t operator()(const Raito::UUID& uuid) const
		{
			return (u64)uuid;
		}
	};

}