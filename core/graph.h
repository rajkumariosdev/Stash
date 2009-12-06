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

#ifndef GRAPH_H
#define GRAPH_H

#include "datetime.h"

enum GraphType
{
	ExpenseCategories,
	ExpensePayees,
	DepositCategories,
	DepositPayees
};

class Graph
{
public:
	Graph();
	~Graph() { };
	
	void setName(std::string name) { m_name = name; }
	void setAccount(int account) { m_account = account; }
	void setStartDate(Date startDate) { m_startDate = startDate; }
	void setEndDate(Date endDate) { m_endDate = endDate; }
	void setType(GraphType type) { m_type = type; }
	
	std::string	getName() { return m_name; }
	int			getAccount() { return m_account; }
	Date		getStartDate() { return m_startDate; }
	Date		getEndDate() { return m_endDate; }
	const Date &getStartDate1() { return m_startDate; }
	const Date &getEndDate1() { return m_endDate; }
	GraphType	getType() { return m_type; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);
	
protected:
	std::string m_name;
	int			m_account;
	Date		m_startDate;
	Date		m_endDate;
	GraphType	m_type;

};

#endif
