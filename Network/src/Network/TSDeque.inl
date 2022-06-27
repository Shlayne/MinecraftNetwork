namespace net
{
	template<typename Data>
	TSDeque<Data>::~TSDeque()
	{
		Clear();

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_ForceExit = true;
		m_Contidion.notify_one();
	}

	template<typename Data>
	Data& TSDeque<Data>::Front()
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		return m_Deque.front();
	}

	template<typename Data>
	const Data& TSDeque<Data>::Front() const
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		return m_Deque.front();
	}

	template<typename Data>
	Data TSDeque<Data>::PopFront()
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		auto data = std::move(m_Deque.front());
		m_Deque.pop_front();
		return data;
	}

	template<typename Data>
	void TSDeque<Data>::PushFront(const Data& crData)
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		m_Deque.push_front(crData);

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_Contidion.notify_one();
	}

	template<typename Data>
	void TSDeque<Data>::PushFront(Data&& rrData)
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		m_Deque.push_front(std::move(rrData));

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_Contidion.notify_one();
	}

	template<typename Data>
	template<typename... Args>
	Data& TSDeque<Data>::EmplaceFront(Args&&... rrArgs)
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		Data& rData = m_Deque.emplace_front(std::forward<Args>(rrArgs)...);

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_Contidion.notify_one();

		return rData;
	}

	template<typename Data>
	Data& TSDeque<Data>::Back()
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		return m_Deque.back();
	}

	template<typename Data>
	const Data& TSDeque<Data>::Back() const
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		return m_Deque.back();
	}

	template<typename Data>
	Data TSDeque<Data>::PopBack()
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		auto data = std::move(m_Deque.back());
		m_Deque.pop_back();
		return data;
	}

	template<typename Data>
	void TSDeque<Data>::PushBack(const Data& crData)
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		m_Deque.push_back(crData);

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_Contidion.notify_one();
	}

	template<typename Data>
	void TSDeque<Data>::PushBack(Data&& rrData)
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		m_Deque.push_back(std::move(rrData));

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_Contidion.notify_one();
	}

	template<typename Data>
	template<typename... Args>
	Data& TSDeque<Data>::EmplaceBack(Args&&... rrArgs)
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		Data& rData = m_Deque.emplace_back(std::forward<Args>(rrArgs)...);

		std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
		m_Contidion.notify_one();

		return rData;
	}

	template<typename Data>
	size_t TSDeque<Data>::Size() const
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		return m_Deque.size();
	}

	template<typename Data>
	bool TSDeque<Data>::Empty() const
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		return m_Deque.empty();
	}

	template<typename Data>
	void TSDeque<Data>::Clear()
	{
		std::scoped_lock<std::mutex> dequeLock(m_DequeMutex);
		m_Deque.clear();
	}

	template<typename Data>
	void TSDeque<Data>::Wait()
	{
		while (Empty() && !m_ForceExit)
		{
			std::unique_lock<std::mutex> conditionLock(m_ContidionMutex);
			m_Contidion.wait(conditionLock);
		}
	}
}
