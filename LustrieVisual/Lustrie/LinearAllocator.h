#pragma once

#include <type_traits>
#include <list>

using u32 = unsigned int;

template<u32 PageSize, u32 Alignement = 8>
class LinearAllocator
{
	static_assert(PageSize % Alignement == 0, "PageSize must be divisible by Alignement.");

public:
	LinearAllocator() : m_curPage{ &m_page0 } {}

	template<typename T, typename... Args>
	T* alloc(Args&& _args...)
	{
		if(m_page0)

	}

private:
	struct Page
	{
		std::aligned_storage<PageSize, Alignement>::type m_data;
		u32 m_offset = 0;
	};

	Page m_page0;
	std::list<Page> m_pages1_n;
	Page* m_curPage = nullptr;
};