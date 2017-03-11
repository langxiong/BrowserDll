#include "StdAfx.h"
#include "MAMethod.h"
#include <numeric>

namespace MyAlgorithm
{
	CSMAMethod::CSMAMethod(const std::vector<double>& initQueue) :
		m_queue(initQueue),
		m_totalVal(0.0f)
	{
		m_totalVal = std::accumulate(std::begin(m_queue), std::end(m_queue), m_totalVal);
	}
	
	double CSMAMethod::Update(double newVal)
	{
		auto firstIt = m_queue.begin();
		double firstVal = *firstIt;
		m_queue.erase(firstIt);
		m_queue.push_back(newVal);
		m_totalVal = m_totalVal - firstVal + newVal; 
		return GetAverageVal();
	}
	
	double CSMAMethod::GetAverageVal() const
	{
		return m_totalVal / m_queue.size();
	}	
	
	CEMAMethod::CEMAMethod(const std::vector<double>& initQueue) :
		m_multiplier(2.0f/(initQueue.size() + 1)),
		m_EMAVal(0.0f)
	{
		double totalVal = 0.0f;
		totalVal = std::accumulate(std::begin(initQueue), std::end(initQueue), totalVal);
		m_EMAVal = totalVal / initQueue.size();
	}
	
	double CEMAMethod::Update(double newVal)
	{
		m_EMAVal = m_multiplier * (newVal - m_EMAVal) + m_EMAVal;
		return GetAverageVal();
	}
	
	double CEMAMethod::GetAverageVal() const
	{
		return m_EMAVal;
	}
}
