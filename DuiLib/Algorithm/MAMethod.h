#pragma once

#include <vector>

namespace MyAlgorithm
{
	class CSMAMethod
	{
	public:
		CSMAMethod(const std::vector<double>& initQueue);
	
		double Update(double newVal);
	
		double GetAverageVal() const;
	
	private:
		std::vector<double> m_queue;
		double m_totalVal;
	};
	
	class CEMAMethod
	{
	public:
		CEMAMethod(const std::vector<double>& initQueue);
	
		double Update(double newVal);
	
		double GetAverageVal() const;
	
	private:
		double m_multiplier;
		double m_EMAVal;
	};
}
