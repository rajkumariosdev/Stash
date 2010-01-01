/* 
 * Stash:  A Personal Finance app for OS X.
 * Copyright (C) 2009 Peter Pearson
 * You can view the complete license in the Licence.txt file in the root
 * of the source tree.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include "analysis.h"
#include "transaction.h"

PieChartItem::PieChartItem(std::string title, double angle, fixed amount) : m_title(title), m_angle(angle), m_amount(amount)
{

}

AreaChartItem::AreaChartItem(std::string title) : m_title(title), m_maxValue(0.0), m_blank(true), m_activeEntries(0)
{

}

void AreaChartItem::addAmountToValue(fixed amount)
{	
	m_blank = false;
	
	m_amounts.push_back(amount);
	
	if (!amount.IsZero())
		m_activeEntries++;
	
	if (m_maxValue < amount)
		m_maxValue = amount;
}

void AreaChartItem::combineItem(AreaChartItem &item)
{
	std::vector<fixed>::iterator thisIt = m_amounts.begin();
	std::vector<fixed>::iterator itemIt = item.m_amounts.begin();
	
	if (m_blank)
	{
		for (; itemIt != item.m_amounts.end(); ++itemIt)
		{
			fixed &itemValue = (*itemIt);
			
			if (!itemValue.IsZero())
				m_activeEntries++;
			
			m_amounts.push_back(itemValue);
		}
		
		m_blank = false;
	}
	else
	{
		for (; thisIt != m_amounts.end(); ++thisIt, ++itemIt)
		{
			fixed &thisValue = (*thisIt);
			
			fixed &itemValue = (*itemIt);
			
			thisValue += itemValue;
		}
		
		if (m_activeEntries < item.m_activeEntries)
			m_activeEntries = item.m_activeEntries;
	}
}

bool buildPieChartItemsForCategories(PieChartCriteria &criteria, bool expense)
{
	if (!criteria.m_pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::iterator it = criteria.m_pAccount->begin();

	for (; it != criteria.m_pAccount->end(); ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if (expense && (*it).getAmount().IsPositive() ||
			!expense && !(*it).getAmount().IsPositive())
		{
			continue;
		}
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string category = split.getCategory();
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itFind = aMap.find(category);
				
				if (itFind == aMap.end())
				{
					aMap[category] = amount;
				}
				else
				{
					fixed &catTotal = (*itFind).second;
					
					catTotal += amount;			
				}
				
				criteria.m_overallTotal += amount;
			}			
		}
		else
		{
			std::string category = (*it).getCategory();
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itFind = aMap.find(category);
			
			if (itFind == aMap.end())
			{
				aMap[category] = amount;
			}
			else
			{
				fixed &catTotal = (*itFind).second;
				
				catTotal += amount;			
			}
			
			criteria.m_overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, criteria);

	return true;
}

bool buildPieChartItemsForPayees(PieChartCriteria &criteria, bool expense)
{
	if (!criteria.m_pAccount)
		return false;
	
	// build up map of categories and the cumulative amount for each
	
	std::map<std::string, fixed> aMap;
	std::map<std::string, fixed>::iterator itFind = aMap.end();
	
	std::vector<Transaction>::iterator it = criteria.m_pAccount->begin();
	
	for (; it != criteria.m_pAccount->end(); ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if (expense && (*it).getAmount().IsPositive() ||
			!expense && !(*it).getAmount().IsPositive())
		{
			continue;
		}
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string payee = split.getPayee();
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itFind = aMap.find(payee);
				
				if (itFind == aMap.end())
				{
					aMap[payee] = amount;
				}
				else
				{
					fixed &payeeTotal = (*itFind).second;
					
					payeeTotal += amount;			
				}
				
				criteria.m_overallTotal += amount;
			}			
		}
		else
		{
			std::string payee = (*it).getPayee();
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itFind = aMap.find(payee);
			
			if (itFind == aMap.end())
			{
				aMap[payee] = amount;
			}
			else
			{
				fixed &payeeTotal = (*itFind).second;
				
				payeeTotal += amount;			
			}
			
			criteria.m_overallTotal += amount;
		}
	}
	
	copyPieItemsToVector(aMap, criteria);
	
	return true;	
}

bool buildAreaChartItemsForCategories(AreaChartCriteria &criteria, bool expense)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::iterator it = criteria.m_pAccount->begin();
	
	for (; it != criteria.m_pAccount->end(); ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if (expense && (*it).getAmount().IsPositive() ||
			!expense && !(*it).getAmount().IsPositive())
		{
			continue;
		}
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
	
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string category = split.getCategory();
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itDateTotal = aDateTotals.find(my);
				
				if (itDateTotal == aDateTotals.end())
				{
					aDateTotals[my] = amount;
				}
				else
				{
					fixed &dateTotal = (*itDateTotal).second;
					
					dateTotal += amount;
				}
				
				itItemFind = aItemMap.find(category);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[category] = dateMap;
				}
				else
				{
					std::map<MonthYear, fixed> &dateMap = (*itItemFind).second;
					
					std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(my);
					
					if (itDateFind == dateMap.end())
					{
						dateMap[my] = amount;						
					}
					else
					{
						fixed &itemTotal = (*itDateFind).second;
						
						itemTotal += amount;
					}		
				}
			}			
		}
		else
		{
			std::string category = (*it).getCategory();
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itDateTotal = aDateTotals.find(my);
			
			if (itDateTotal == aDateTotals.end())
			{
				aDateTotals[my] = amount;
			}
			else
			{
				fixed &dateTotal = (*itDateTotal).second;
				
				dateTotal += amount;
			}
			
			itItemFind = aItemMap.find(category);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[category] = dateMap;
			}
			else
			{
				std::map<MonthYear, fixed> &dateMap = (*itItemFind).second;
				
				std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(my);
				
				if (itDateFind == dateMap.end())
				{
					dateMap[my] = amount;				
				}
				else
				{
					fixed &itemTotal = (*itDateFind).second;
					
					itemTotal += amount;
				}
			}
		}
	}

	copyAreaItemsToVector(aItemMap, aDateTotals, criteria);
	
	return true;
}

bool buildAreaChartItemsForPayees(AreaChartCriteria &criteria, bool expense)
{
	// make temporary cache of all items on a month/day basis
	std::map<MonthYear, fixed> aDateMap;
	std::map<std::string, std::map< MonthYear, fixed > > aItemMap;
	
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItemFind = aItemMap.end();
	std::map<MonthYear, fixed>::iterator itDateFind = NULL;
	
	std::map<MonthYear, fixed> aDateTotals;
	std::map<MonthYear, fixed>::iterator itDateTotal = NULL;
	
	std::vector<Transaction>::iterator it = criteria.m_pAccount->begin();
	
	for (; it != criteria.m_pAccount->end(); ++it)
	{
		if ((*it).getDate() < criteria.m_startDate || (*it).getDate() > criteria.m_endDate)
			continue;
		
		if (expense && (*it).getAmount().IsPositive() ||
			!expense && !(*it).getAmount().IsPositive())
		{
			continue;
		}
		
		if (criteria.m_ignoreTransfers && (*it).getType() == Transfer)
			continue;
		
		MonthYear my((*it).getDate().getMonth(), (*it).getDate().getYear());		
		
		if ((*it).isSplit() && (*it).getSplitCount() > 0)
		{
			for (int i = 0; i < (*it).getSplitCount(); i++)
			{
				SplitTransaction &split = (*it).getSplit(i);
				
				std::string payee = split.getPayee();
				fixed amount = split.getAmount();
				
				amount.setPositive();
				
				itDateTotal = aDateTotals.find(my);
				
				if (itDateTotal == aDateTotals.end())
				{
					aDateTotals[my] = amount;
				}
				else
				{
					fixed &dateTotal = (*itDateTotal).second;
					
					dateTotal += amount;
				}
				
				itItemFind = aItemMap.find(payee);
				
				if (itItemFind == aItemMap.end())
				{
					std::map<MonthYear, fixed> dateMap;
					
					dateMap[my] = amount;
					
					aItemMap[payee] = dateMap;
				}
				else
				{
					std::map<MonthYear, fixed> &dateMap = (*itItemFind).second;
					
					std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(my);
					
					if (itDateFind == dateMap.end())
					{
						dateMap[my] = amount;						
					}
					else
					{
						fixed &itemTotal = (*itDateFind).second;
						
						itemTotal += amount;
					}		
				}
			}			
		}
		else
		{
			std::string payee = (*it).getPayee();
			fixed amount = (*it).getAmount();
			
			amount.setPositive();
			
			itDateTotal = aDateTotals.find(my);
			
			if (itDateTotal == aDateTotals.end())
			{
				aDateTotals[my] = amount;
			}
			else
			{
				fixed &dateTotal = (*itDateTotal).second;
				
				dateTotal += amount;
			}
			
			itItemFind = aItemMap.find(payee);
			
			if (itItemFind == aItemMap.end())
			{
				std::map<MonthYear, fixed> dateMap;
				
				dateMap[my] = amount;
				
				aItemMap[payee] = dateMap;
			}
			else
			{
				std::map<MonthYear, fixed> &dateMap = (*itItemFind).second;
				
				std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(my);
				
				if (itDateFind == dateMap.end())
				{
					dateMap[my] = amount;				
				}
				else
				{
					fixed &itemTotal = (*itDateFind).second;
					
					itemTotal += amount;
				}
			}
		}
	}
	
	copyAreaItemsToVector(aItemMap, aDateTotals, criteria);
	
	return true;
}

void copyPieItemsToVector(std::map<std::string, fixed> &aMap, PieChartCriteria &criteria)
{
	double dOverallTotal = criteria.m_overallTotal.ToDouble();
	
	fixed leftovers = 0.0;
	
	// copy values to the vector
	
	std::map<std::string, fixed>::iterator itMap = aMap.begin();
	
	double dGroupSmaller = static_cast<double>(criteria.m_groupSmaller);
	
	for (; itMap != aMap.end(); ++itMap)
	{
		std::string title = (*itMap).first;
		
		fixed amount = (*itMap).second;
		
		if (title.empty() || title == criteria.m_groupSmallerName)
		{
			leftovers += amount;
			continue;
		}
		
		double dPieAngle = (amount.ToDouble() / dOverallTotal) * 360.0;
		
		if (criteria.m_groupSmaller == -1 || dPieAngle > dGroupSmaller)
		{
			PieChartItem newGraphValue(title, dPieAngle, amount);
			
			criteria.m_aValues.push_back(newGraphValue);
		}
		else
		{
			leftovers += amount;
		}
	}
	
	// sort the values
	
	if (criteria.m_eSort == PieChartSortTitle)
	{
		std::sort(criteria.m_aValues.begin(), criteria.m_aValues.end(), PieChartItem::PieChartSortTitle);
	}
	else
	{
		std::sort(criteria.m_aValues.begin(), criteria.m_aValues.end(), PieChartItem::PieChartSortAngle);
	}
	
	// add other category with any leftovers that are too small to bother showing
	
	if (!leftovers.IsZero())
	{
		double dPieAngle = (leftovers.ToDouble() / dOverallTotal) * 360.0;
		
		PieChartItem newGraphValue(criteria.m_groupSmallerName, dPieAngle, leftovers);
		
		criteria.m_aValues.push_back(newGraphValue);
	}	
}

void copyAreaItemsToVector(std::map<std::string, std::map< MonthYear, fixed > > &aMap, std::map<MonthYear, fixed> &aDateTotals, AreaChartCriteria &criteria)
{
	//	for each item, for each date that exists in the aDates set, create an AreaChartItem for that date and the item
		
	std::map<std::string, std::map< MonthYear, fixed > >::iterator itItem = aMap.begin();
	
	for (; itItem != aMap.end(); ++itItem)
	{
		std::string title = (*itItem).first;
		
		std::map<MonthYear, fixed>::iterator itDate = aDateTotals.begin();
		
		AreaChartItem newItem(title);
		
		for (; itDate != aDateTotals.end(); ++itDate)
		{
			std::map<MonthYear, fixed> &dateMap = (*itItem).second;
			
			MonthYear myDate = (*itDate).first;
			
			fixed &dateTotal = (*itDate).second;
			
			if (criteria.m_overallMax < dateTotal)
				criteria.m_overallMax = dateTotal;
			
			std::map<MonthYear, fixed>::iterator itDateFind = dateMap.find(myDate);
			
			if (itDateFind == dateMap.end())
			{
				newItem.addAmountToValue(0.0);				
			}
			else
			{
				fixed &amount = (*itDateFind).second;
				
				newItem.addAmountToValue(amount);
			}
		}
		
		criteria.m_aValues.push_back(newItem);
	}
	
	// now work out what's 2% of the max overall value and if the item's less than that, remove it
	
	double dPurgeValue = criteria.m_overallMax.ToDouble();
	dPurgeValue *= 0.02;
	
	bool bAddOther = false;
	AreaChartItem otherItem("Other");
	
	std::vector<AreaChartItem>::iterator itPurgeItem = criteria.m_aValues.begin();
	
	while (itPurgeItem != criteria.m_aValues.end())
	{
		if ((*itPurgeItem).getMaxValue().ToDouble() < dPurgeValue)
		{
			otherItem.combineItem(*itPurgeItem);
			itPurgeItem = criteria.m_aValues.erase(itPurgeItem);
			bAddOther = true;
		}
		else
		{
			++itPurgeItem;
		}
	}
	
	if (bAddOther)
		criteria.m_aValues.push_back(otherItem);
	
	// sort the items so that items with fewer actual values (most likely occasional expenditures) get done last
	// so they won't affect the more regular items, and will stand out more 
	std::sort(criteria.m_aValues.begin(), criteria.m_aValues.end());
		
	// add dateitems to vector
	std::map<MonthYear, fixed>::iterator itDate2 = aDateTotals.begin();
	
	for (; itDate2 != aDateTotals.end(); ++itDate2)
	{
		MonthYear myDate = (*itDate2).first;
		
		criteria.m_aDates.push_back(myDate);		
	}
}