
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <cstdint>
#include <condition_variable>
#include <future>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////
class Base
{
protected:
	//int8_t* tasks;
	vector<int8_t> tasks;
	size_t taskSize;
public:
	virtual void incMass(bool isDelay) {};
	virtual int returnAndIncrement() { return 0; };
};


class MutexCounter :public Base
{

private:
	int value;
	mutex _mutex;

public:

	MutexCounter(vector<int8_t> tasks, size_t taskSize)
	{
		value = 0;
		this->tasks = tasks;
		this->taskSize = taskSize;
	}

	int returnAndIncrement() override
	{
		lock_guard<mutex> locker(_mutex);
		return value++;
	}

	void incMass(bool isDelay) override
	{
		size_t i;
		while ((i = returnAndIncrement()) < taskSize)
		{
			tasks.at(i)++;
			if (isDelay)
				this_thread::sleep_for(std::chrono::nanoseconds(10));
		}
	}
};


class AtomicCounter :public Base
{

private:
	atomic<int> value;

public:

	AtomicCounter(vector<int8_t> tasks, size_t taskSize)
	{
		value.store(0);
		this->tasks = tasks;
		this->taskSize = taskSize;
	}

	int returnAndIncrement() override
	{
		return value.fetch_add(1);
	}

	void incMass(bool isDelay)  override
	{
		size_t i;
		while ((i = returnAndIncrement()) < taskSize)
		{
			tasks.at(i)++;
			if (isDelay)
				this_thread::sleep_for(std::chrono::nanoseconds(10));
		}
	}
};


//void check(int8_t* mass, size_t size)
//{
//	size_t i;
//	bool isOk = true;
//	for (i = 0; i < size; i++)
//	{
//		if (mass[i] != 1)
//		{
//			isOk = false;
//			break;
//		}
//	}
//	if (isOk)
//		cout << "\nOk";
//	else
//		cout << "\nFail";
//}
void task1_(Base* counter, int  thrCount, bool isDelay)
{
	auto start = chrono::high_resolution_clock::now();
	vector<thread> thrds;
	thrds.clear();
	for (int i = 0; i < thrCount; i++)
	{
		thread thr(&Base::incMass, counter, isDelay);
		thrds.push_back(move(thr));
	}
	for (int i = 0; i < thrCount; i++)
	{
		if (thrds[i].joinable())
			thrds[i].join();
	}
	auto end = chrono::high_resolution_clock::now();

	cout << "\nThread count:" << thrCount << "\n" << "Time:" << chrono::duration_cast<chrono::milliseconds>(end - start).count();

}
void task1()
{
	vector<int> thread_count{ 4,8,16,32 };
	long long size = 1024 * 1024;
	cout << "\n\nMutex Without delay:";
	for (auto c : thread_count)
	{
		vector<int8_t> mass(size, { 0 });// = new int8_t[size]{ 0 };
		auto mutex_ = new MutexCounter(mass, size);
		task1_(mutex_, c, false);
		//check(mass, size);
		delete mutex_;
		//delete mass;
	}
	cout << "\n\nMutex With delay:";
	for (auto c : thread_count)
	{
		vector<int8_t> mass(size, { 0 });// = new int8_t[size]{ 0 };
		auto mutex_ = new MutexCounter(mass, size);
		task1_(mutex_, c, true);
		//check(mass, size);
		delete mutex_;
		//delete[] mass;
	}
	cout << "\n\n\n";
	cout << "Atomic Without delay:";
	for (auto c : thread_count)
	{
		vector<int8_t> mass(size, { 0 });// = new int8_t[size]{ 0 };
		auto atomic_ = new AtomicCounter(mass, size);
		task1_(atomic_, c, false);
		//check(mass, size);
		delete atomic_;
		//delete mass;
	}
	cout << "\n\nAtomic With delay:";
	for (auto c : thread_count)
	{
		vector<int8_t> mass(size, { 0 });// = new int8_t[size]{ 0 };
		auto atomic_ = new AtomicCounter(mass, size);
		task1_(atomic_, c, true);
		//check(mass, size);
		delete atomic_;
		//delete[] mass;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////

class Queue
{
protected:
	queue<uint8_t> ourQueue;
	mutex mutex_;
public:
	virtual void Push(uint8_t value) {};
	virtual bool Pop(uint8_t& value) { return 0; };
};



class DynamicQueue : public Queue
{
public:
	void Push(uint8_t value) override
	{
		lock_guard<mutex> locker(this->mutex_);
		this->ourQueue.push(value);
	}

	bool Pop(uint8_t& value) override
	{
		//lock_guard<mutex> locker(this->mutex_);
		unique_lock<mutex> locker(this->mutex_);
		if (this->ourQueue.empty())
		{
			locker.unlock();

			this_thread::sleep_for(chrono::milliseconds(1));

			if (this->ourQueue.empty())
			{
				return false;
			}
			locker.lock();
		}

		value = ourQueue.front();
		this->ourQueue.pop();
		locker.unlock();
		return true;
	}
};



class FixQueue : public Queue
{
private:
	condition_variable rd, wr;
	size_t size;

public:

	FixQueue(size_t size)
	{
		this->size = size;
	}

	void Push(uint8_t value) override
	{
		unique_lock<mutex> locker(this->mutex_);

		if (this->ourQueue.size() >= this->size)
		{
			this->wr.wait(locker);
		}

		this->ourQueue.push(value);
		this->rd.notify_all();
	}

	bool Pop(uint8_t& value) override
	{
		unique_lock<mutex> locker(this->mutex_);
		if (this->ourQueue.empty())
			this->rd.wait_for(locker, 1ms);
		if (this->ourQueue.empty())
			return false;

		value = (this->ourQueue).front();
		this->ourQueue.pop();

		this->wr.notify_all();
		return true;

	}
};



class TestData
{
public:
	size_t taskNum;
	int prodNum;
	int consNum;
	Queue* queue;
};

void SProducer(TestData data)
{
	for (int i = 0; i < data.taskNum; i++)
	{
		data.queue->Push(1);
	}
}

void SConsumer(TestData data, atomic<int>& sum)
{
	uint8_t value;
	while (sum.load() < data.taskNum * data.prodNum )
	{
		if (data.queue->Pop(value))
		{
			sum.fetch_add(value);
		}
	}
}


void task2()
{
	vector<TestData> data =
	{
		{ 1024,1,1,new FixQueue(1)},
		{ 1024 * 10,2,2,new FixQueue(4)},
		{ 1024 * 102 , 4,4, new FixQueue(16)},
		{ 1024 ,1,1,new DynamicQueue()},
		{ 1024 ,2,2,new DynamicQueue()},
		{ 1024 * 10 , 4,4, new DynamicQueue()},
	};

	for (int j = 0; j < 6; j++)
	{
		thread* cons = new thread[data[j].consNum];
		thread* prod = new thread[data[j].prodNum];
		atomic<int> sum;
		sum.store(0);
		auto start = chrono::steady_clock::now();


		for (int i = 0; i < data[j].prodNum; i++)
		{
			prod[i] = thread(SProducer, data[j]);
			cons[i] = thread(SConsumer, data[j], ref(sum));
		}

		for (int i = 0; i < data[j].prodNum; i++)
		{
			prod[i].join();
			cons[i].join();
		}
		auto end = chrono::steady_clock::now();

		if (sum.load() == data[j].taskNum * data[j].prodNum)
			cout << "\n" << "Time: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "\n";
		delete[] cons;
		delete[] prod;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	task2();
	task1();
}
