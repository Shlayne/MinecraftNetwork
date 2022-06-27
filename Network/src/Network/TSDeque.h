#pragma once

#include "Network/Common.h"

namespace net
{
	// Thread-Safe Deque
	template<typename Data>
	class TSDeque
	{
	public:
		TSDeque() = default;
		~TSDeque();
		TSDeque(const TSDeque<Data>&) = delete;
		TSDeque& operator=(const TSDeque<Data>&) = delete;
	public:
		Data& Front();
		const Data& Front() const;
		Data PopFront();
		void PushFront(const Data& crData);
		void PushFront(Data&& rrData);
		template<typename... Args>
		Data& EmplaceFront(Args&&... rrArgs);
	public:
		Data& Back();
		const Data& Back() const;
		Data PopBack();
		void PushBack(const Data& crData);
		void PushBack(Data&& rrData);
		template<typename... Args>
		Data& EmplaceBack(Args&&... rrArgs);
	public:
		size_t Size() const;
		bool Empty() const;
		void Clear();
	public:
		void Wait();
	private:
		mutable std::mutex m_DequeMutex;
		std::deque<Data> m_Deque;
		std::mutex m_ContidionMutex;
		std::condition_variable m_Contidion;
		std::atomic_bool m_ForceExit = false;
	};
}

#include "Network/TSDeque.inl"
