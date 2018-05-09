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
		const std::lock_guard<std::mutex> lg(m_mutex);
		m_slots.push_back(sharedSlot);
		return Connection(std::move(sharedSlot));
	}

	Connection Connect(SlotFn&& slot)
	{
		std::shared_ptr<SlotFn> sharedSlot = std::make_shared<SlotFn>(std::move(slot));
		const std::lock_guard<std::mutex> lg(m_mutex);
		m_slots.push_back(sharedSlot);
		return Connection(std::move(sharedSlot));
	}

	void Emit(Args... params)
	{
		std::vector<std::shared_ptr<SlotFn>> funcs;
		std::vector<std::weak_ptr<SlotFn>> validSlots;
		{
			const std::lock_guard<std::mutex> lg(m_mutex);
			validSlots.reserve(m_slots.size());
			for (auto& slot : m_slots)
			{
				if (auto sharedFn = slot.lock())
				{
					funcs.push_back(std::move(sharedFn));
					validSlots.push_back(std::move(slot));
				}
			}
			m_slots = std::move(validSlots);
		}
		for (const auto& slotFun : funcs)
		{
			(*slotFun)(params...);
		}
	}

private:

	std::vector<std::weak_ptr<SlotFn>> m_slots;
	std::mutex m_mutex;
};
} // namespace SimpleSignals
