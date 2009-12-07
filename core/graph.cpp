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

#include <bitset>
#include "graph.h"
#include "string.h"

Graph::Graph() : m_name(""), m_account(0), m_type(ExpenseCategories), m_ignoreTransfers(false)
{

}

void Graph::Load(std::fstream &stream, int version)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 512 accounts...
	stream.read((char *) &m_account, sizeof(unsigned char));
	
	LoadString(m_name, stream);
	
	m_startDate.Load(stream, version);
	m_endDate.Load(stream, version);
	
	stream.read((char *) &m_type, sizeof(unsigned char));
	
	unsigned char cBitset = 0;
	stream.read((char *) &cBitset, sizeof(unsigned char));
	
	std::bitset<8> localset(static_cast<unsigned long>(cBitset));
	
	m_ignoreTransfers = localset[0];
}

void Graph::Store(std::fstream &stream)
{
	// not the best way of doing this, should have some unique ID for accounts...
	// not strictly valid either, but are we going to have more than 512 accounts...
	stream.write((char *) &m_account, sizeof(unsigned char));
	
	StoreString(m_name, stream);
	
	m_startDate.Store(stream);
	m_endDate.Store(stream);
	
	stream.write((char *) &m_type, sizeof(unsigned char));
	
	std::bitset<8> localset;	
	localset[0] = m_ignoreTransfers;
	
	unsigned char cBitset = static_cast<unsigned char>(localset.to_ulong());
	stream.write((char *) &cBitset, sizeof(unsigned char));
}