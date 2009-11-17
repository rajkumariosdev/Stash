#ifndef SPLITTRANSACTION_H
#define SPLITTRANSACTION_H

/*
 *  split_transaction.h
 *  money
 *
 *  Created by Peter Pearson on 23/09/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "fixed.h"
#include <string>

class SplitTransaction
{
public:
	SplitTransaction() { }
	SplitTransaction(std::string Description, std::string Payee, std::string Category, fixed Amount);
	
	std::string Description() const { return m_Description; }
	void setDescription(std::string Description) { m_Description = Description; }
	std::string Payee() const { return m_Payee; }
	void setPayee(std::string Payee) { m_Payee = Payee; }
	std::string Category() const { return m_Category; }
	void setCategory(std::string Category) { m_Category = Category; }
	fixed Amount() const { return m_Amount; }
	void setAmount(fixed Amount) { m_Amount = Amount; }
	
	void Load(std::fstream &stream, int version);
	void Store(std::fstream &stream);

private:
	
	std::string m_Description;
	std::string m_Payee;
	std::string m_Category;
	fixed m_Amount;

};
#endif SPLITTRANSACTION_H