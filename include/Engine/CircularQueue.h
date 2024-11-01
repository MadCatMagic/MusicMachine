#pragma once
#include <utility>
#include <cassert>

// T must have empty constructer
template <typename T> class CircularQueue
{
public:
	inline CircularQueue(size_t s) : _size(s)
	{
		data = new T[_size];
	}

	// copy constructor
	inline CircularQueue(const CircularQueue<T>& q)
	{
		head = q.head;
		_length = q._length;
		_size = q._size;
		data = new T[_size];
		for (size_t i = 0; i < _length; i++)
		{
			size_t index = (i + head) % _length;
			data[index] = q.data[index];
		}
	}

	// move constructor
	inline CircularQueue(CircularQueue<T>&& q)
	{
		head = q.head;
		_length = q._length;
		_size = q._size;
		data = new T[_size];
		for (size_t i = 0; i < _length; i++)
		{
			size_t index = (i + head) % _length;
			data[index] = std::move(q.data[index]);
		}

		q.~CircularQueue();
	}
	
	// copy assignment operator
	inline CircularQueue<T>& operator=(const CircularQueue<T>& q)
	{
		head = q.head;
		_length = q._length;
		_size = q._size;
		data = new T[_size];
		for (size_t i = 0; i < _length; i++)
		{
			size_t index = (i + head) % _length;
			data[index] = q.data[index];
		}
		return *this;
	}

	// move assignment operator
	inline CircularQueue<T>& operator=(CircularQueue<T>&& q)
	{
		head = q.head;
		_length = q._length;
		_size = q._size;
		data = new T[_size];
		for (size_t i = 0; i < _length; i++)
		{
			size_t index = (i + head) % _length;
			data[index] = std::move(q.data[index]);
		}

		q.~CircularQueue();
		return *this;
	}

	inline ~CircularQueue()
	{
		delete[] data;
	}

	// copy push
	inline void push(const T& value) 
	{ 
		assert(_length < _size);
		data[(head + _length) % _size] = value; 
		_length++;
	}

	// move push
	inline void push(T&& value) {
		assert(_length < _size);
		data[(head + _length) % _size] = std::move(value);
		_length++;
	}

	// pop from front
	inline T pop()
	{
		assert(_length > 0);
		size_t popv = head;
		head = (head + 1) % _size;
		_length--;
		return move(data[popv]);
	}

	// peek at front
	inline T& peek()
	{
		assert(length > 0);
		return &data[head];
	}

	// length of current queue
	inline size_t length() const { return _length; }
	// total available space
	inline size_t size() const { return _size; }
	// whether the queue is empty
	inline bool empty() const { return _length == 0; }
	// whether the queue is full
	inline bool full() const { return _length == _size; }

private:
	// pointer to beginning of data in memory
	T* data;
	// offset to current head of queue
	size_t head = 0;
	// occupied length of queue
	size_t _length = 0;
	// total available size of queue
	size_t _size;
};