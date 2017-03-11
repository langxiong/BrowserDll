#pragma once

#include <vector>
#include <utility>

namespace MyAlgorithm
{
	class CRSIMethod
	{
	public:
		CRSIMethod(const std::vector<std::pair<double, double>>& initQueue);

		double Update(const std::pair<double, double>& newVal);

		double GetRSVal() const;

		double GetAverageVal() const;

	private:
		std::vector<std::pair<double, double>> m_queue;
		double m_gains;
		double m_losses;
	};

}
