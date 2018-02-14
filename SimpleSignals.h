/*
The MIT License(MIT)

Copyright 2017 Steffen Keller

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <vector>

namespace SimpleSignals
{
class Connection
{
public:
	Connection(std::shared_ptr<void>&& slotPtr)
		:m_slotptr(std::move(slotPtr))
	{}

	void Disconnect()
	{
		m_slotptr = nullptr;
	}
private:
	std::shared_ptr<void> m_slotptr;
};

template <typename...Args>
class Signal
{
public:
	using SlotFn = std::function<void(Args...)>;

	Connection Connect(const SlotFn& slot)
	{
		std::shared_ptr<SlotFn> sharedSlot = std::make_shared<SlotFn>(slot);
		ConnectInternal(sharedSlot);
		return Connection(std::move(sharedSlot));
	}

	Connection Connect(SlotFn&& slot)
	{
		std::shared_ptr<SlotFn> sharedSlot = std::make_shared<SlotFn>(std::move(slot));
		ConnectInternal(sharedSlot);
		return Connection(std::move(sharedSlot));
	}

	void Emit(Args... params)
	{
		std::vector<int32_t> idToErase;
		for (const auto& slot : m_slots)
		{
			if (const auto sharedFn = slot.second.lock())
			{
				(*sharedFn)(params...);
			}
			else
			{
				idToErase.push_back(slot.first);
			}
		}
		if (idToErase.empty())
			return;

		// remove dead connections
		std::lock_guard<std::mutex> lg(m_mutex);
		for (const auto& cid : idToErase)
		{
			m_slots.erase(cid);
		}
	}

private:

	void ConnectInternal(const std::shared_ptr<SlotFn>& sharedSlot)
	{
		std::lock_guard<std::mutex> lg(m_mutex);
		m_slots[++m_slotId] = sharedSlot;
	}

	int32_t m_slotId = 0;
	std::map<int32_t, std::weak_ptr<SlotFn>> m_slots;
	std::mutex m_mutex;
};
} // namespace SimpleSignals
