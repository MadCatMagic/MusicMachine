#pragma once
#include "Vector.h"
#include <vector>
#include <deque>

class AudioStream
{
public:
	inline void SetData(std::vector<v2>& v)
	{
		if (dataAFirst)
		{
			if (dataA.size() == 0)
				dataA = v;
			else
				dataB = v;
		}
		else
		{
			if (dataB.size() == 0)
				dataB = v;
			else
				dataA = v;
		}
	}

	inline bool NoData() const
	{
		return dataA.size() == 0 && dataB.size() == 0;
	}

	std::vector<v2> dataA;
	std::vector<v2> dataB;

	bool dataAFirst = true;

	float previousData[1024]{};
	unsigned int previousDataP = 0;
};