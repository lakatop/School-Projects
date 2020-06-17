#ifndef _STENCIL1D_HPP
#define _STENCIL1D_HPP

#include <thread>
#include <cstddef>
#include <vector>
#include <mutex>
#include <condition_variable>

//forward declaration
template <typename ET>
class PartialHolder;

class Lock
{
public:
	Lock(size_t generations, size_t thrs) : genCount(generations), thrCount(thrs), thrCompare(thrs) {}
	void Wait()
	{
		std::unique_lock l(m);
		thrCount--;
		auto comp = genCount;
		if (thrCount == 0)
		{
			genCount--;
			thrCount = thrCompare;
			condVar.notify_all();
		}
		else
			while (genCount == comp)
				condVar.wait(l);
	}
private:
	std::mutex m;
	std::condition_variable condVar;
	size_t genCount;
	size_t thrCount;
	size_t thrCompare;
};

template <typename ET>
class circle
{
public:
	circle(size_t s) : cells(s) {}
	size_t size() const
	{
		return cells.size();
	}
	void set(std::ptrdiff_t x, const ET& v)
	{
		auto index = (x + size()) % size();
		cells[index] = v;
	}
	const ET get(std::ptrdiff_t x) const
	{
		auto index = (x + size()) % size();
		return cells[index];
	}

	std::vector<ET> cells;
	size_t minThrSize;
	std::vector<PartialHolder<ET>> holders;
	std::vector<size_t> holderSize;
	std::vector<std::thread> threads;

	template <typename SF>
	void run(SF&& sf, size_t g, size_t thrs = std::thread::hardware_concurrency())
	{
		if (thrs >= size())
			thrs = size() / 4 + 1;

		minThrSize = size() / thrs;
		
		//setting computing size for each holder
		int tmpSize = size();
		for (size_t i = 0; i < thrs; i++)
		{
			holderSize.push_back(minThrSize);
			tmpSize -= minThrSize;
		}
		for (size_t i = 0; i < holderSize.size(); i++)
		{
			if (tmpSize == 0)
				break;
			holderSize[i] += 1;
			tmpSize -= 1;
		}

		Lock l(g * 2,thrs);
		size_t overlap = (g < minThrSize / 2) ? g : minThrSize / 2;

		//filling up holders
		int start = 0;
		for (size_t i = 0; i < thrs; i++)
		{
			holders.emplace_back(start, start + holderSize[i], cells, i, &l, this, overlap);
			start += holderSize[i];
		}

		//create new threads and start working
		for (size_t i = 0; i < holders.size(); i++)
		{
			threads.emplace_back(&PartialHolder<ET>::template PartialHolderComputing<SF>, std::ref(holders[i]), std::ref(sf), g);
		}

		//join all threads after work is done
		for (auto&& x : threads)
			x.join();

		//update cells with correct values
		for (size_t i = 0; i < holders.size(); i++)
		{
			size_t index = overlap;
			for (size_t j = holders[i].start_; j < holders[i].end_; j++)
			{
				cells[j] = holders[i].holder[index];
				index++;
			}
		}

		holders.clear();
		threads.clear();
		holderSize.clear();
	}
};


template <typename ET>
class PartialHolder
{
public:
	PartialHolder(size_t start, size_t end, std::vector<ET>& cells, int pos, Lock* l, circle<ET>* c, size_t ov)
	{
		overlap = ov;
		lastValidHolder = true;
		circ = c;
		lock = l;
		start_ = start;
		end_ = end;
		position = pos;
		
		// <st,en) define working scope
		int st = (start + cells.size() - overlap) % cells.size();
		int en = (end + overlap) % cells.size();

		//fill up holder with current cells values
		if (st < en)
			for (int i = st; i < en; i++)
				holder.push_back(cells[i]);
		else
		{
			for (int i = st; i < (int)cells.size(); i++)
				holder.push_back(cells[i]);
			for (int i = 0; i < en; i++)
				holder.push_back(cells[i]);
		}

		buffer = std::vector<ET>(holder.size());
	}

	template <typename SF>
	void PartialHolderComputing(SF&& sf, size_t genCount)
	{
		while (genCount > 0)
		{
			size_t genSize = (((int)genCount - (int)overlap) > 0) ? overlap : genCount;
			if (genSize == overlap)
			{
				PerformGeneration<SF>(genSize, sf);
				lock->Wait();
				Synchronize();
				lock->Wait();
				genCount -= genSize;
			}
			else   //if gensize < overlap, there is no need to synchronize
			{
				PerformGeneration<SF>(genSize, sf);
				genCount -= genSize;
			}
		}
	}

	template <typename SF>
	void PerformGeneration(size_t genSize, SF&& sf)
	{
		for (size_t i = 0; i < genSize; i++)
		{
			for (size_t j = 1 + i; j < holder.size() - i - 1; j++)
			{
				if (lastValidHolder)
				{
					buffer[j] = sf(holder[j - 1], holder[j], holder[j + 1]);
				}
				else
				{
					holder[j] = sf(buffer[j - 1], buffer[j], buffer[j + 1]);
				}
			}
			lastValidHolder = !lastValidHolder;
		}

		if (!lastValidHolder)
		{
			std::swap(holder, buffer);
			lastValidHolder = true;
		}
	}

	void Synchronize()
	{
		PartialHolder<ET>& forward = circ->holders[(position + circ->holders.size() + 1) % circ->holders.size()];
		PartialHolder<ET>& backward = circ->holders[(position + circ->holders.size() - 1) % circ->holders.size()];
		size_t backwardend = backward.holder.size() - (2 * overlap);
		for (size_t i = 0; i < overlap; i++)
		{
			holder[i] = backward.holder[backwardend + i];
			holder[holder.size() - i - 1] = forward.holder[2 * overlap - i - 1];
		}
	}

	size_t start_;
	size_t end_;
	size_t overlap;
	int position;
	std::vector<ET> holder;
	std::vector<ET> buffer;
	bool lastValidHolder;
	Lock* lock;
	circle<ET>* circ;
};

#endif