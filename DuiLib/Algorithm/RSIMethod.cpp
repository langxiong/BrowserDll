#include "StdAfx.h"
#include "RSIMethod.h"
#include <cmath>

namespace MyAlgorithm
{
	CRSIMethod::CRSIMethod(const std::vector<std::pair<double, double>>& initQueue) :
		m_queue(initQueue),
		m_gains(0.0f),
		m_losses(0.0f)
	{
		for (auto it = m_queue.cbegin(); it != m_queue.cend(); ++it)
		{
			m_gains += it->first;
			m_losses += it->second;
		}
	}

	double CRSIMethod::Update(const std::pair<double, double>& newVal)
	{
		auto firstIt = m_queue.begin();
		auto firstVal = *firstIt;
		m_queue.erase(firstIt);
		m_queue.push_back(newVal);
		m_gains = m_gains - firstVal.first + newVal.first; 
		m_losses = m_losses - firstVal.second + newVal.second; 
		return GetAverageVal();
	}

	double CRSIMethod::GetRSVal() const
	{
		return std::abs(m_gains / m_losses);
	}

	double CRSIMethod::GetAverageVal() const
	{
		return 100 - 100 / (1 + GetRSVal());
	}

}
