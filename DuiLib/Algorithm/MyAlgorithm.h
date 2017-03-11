/******************************************************************************
 *  版权所有（C）2012-2016													*
 *  保留所有权利。                                                            *
 *****************************************************************************
 *  作者 : 熊浪
 *  版本 : 1.0
 *****************************************************************************/
/*  修改记录: 
      日期       版本    修改人             修改内容
    --------------------------------------------------------------------------
******************************************************************************/
#pragma once

#include <utility>

namespace MyAlgorithm
{
	// 扩展minmax_element
	template<class ForwardIt, class CompareLess, class CompareGreater>
	std::pair<ForwardIt, ForwardIt> 
		my_minmax_element(ForwardIt first, ForwardIt last, CompareLess compLess, CompareGreater compGreater)
	{
		std::pair<ForwardIt, ForwardIt> result(first, first);

		if (first == last) return result;
		if (++first == last) return result;

		if (compLess(*first, *result.first)) {
			result.first = first;
		} 

		if (compGreater(*first, *result.second)) {
			result.second = first;
		}
		while (++first != last) {
			ForwardIt i = first;
			if (++first == last) {
				if (compLess(*i, *result.first)) result.first = i;
				if (compGreater(*i, *result.second)) result.second = i;
				break;
			} else {
				if (compLess(*first, *i)) {
					if (compLess(*first, *result.first)) result.first = first;
				} else {
					if (compLess(*i, *result.first)) result.first = i;
				}

				if (compGreater(*first, *i)){
					if (compGreater(*first, *result.second)) result.second = first;
				} else {
					if (compGreater(*i, *result.second)) result.second = i;
				}
			}
		}
		return result;
	}
}