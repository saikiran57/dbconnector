// dbconnector.cpp : Defines the entry point for the application.
//

#include "dbconnector.h"

#include "Poco/Exception.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/Statementimpl.h"
#include "Poco/Data/Sessionpool.h"
#include "Poco/Data/Pooledsessionimpl.h"
#include "Poco/Data/Mysql/Connector.h"
#include "Poco/Data/Mysql/Mysqlexception.h"
#include "Poco/Data/Sqlite/Connector.h"
#include "Poco/Data/Sqlite/Sqliteexception.h"
#include "Poco/Data/Sessionpool.h"
#include "Poco/Thread.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/FileStream.h"

#include <iostream>
#include <format>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::File;
using Poco::Path;

constexpr std::string_view FIRST_RULE = "WITH stats as (select t.account_number, t.transaction_number , t.merchant_description, t.transaction_amount , "
" (t.transaction_amount - avg(t.transaction_amount) over()) / (stddev(t.transaction_amount) over()) z_score "
" from customer.transactions t) "
" select concat(ai.` first_name` , ' ', ai.`last_name`) as name, ai.account_number, transaction_number, merchant_description, transaction_amount "
" FROM stats "
" INNER JOIN customer.account_info ai ON stats.account_number = ai.account_number "
" where z_score > 1.645 OR z_score < -1.645 "
" order by account_number";

constexpr std::string_view SECOND_RULE = "select concat(ai.` first_name` , ' ', ai.`last_name`) as name, ai.account_number , t.transaction_number, ai.` city` as expected, merchant_description as Acutal "
" FROM customer.transactions t "
" INNER JOIN customer.account_info ai ON t.account_number = ai.account_number "
" where LOCATE(ai.` city`, t.merchant_description) = 0 "
" order by t.account_number asc;";

int main()
{
	try
	{
		Poco::Data::MySQL::Connector::registerConnector();
		Poco::Data::SessionPool Pool("MySQL", "user=root;password=password;d b=calserver;compress=true;auto-reconnect=true", 1, 4, 5);
		Poco::Data::Session sess(Pool.get());

		if (sess.isConnected())
		{
			{
				typedef Poco::Tuple<std::string, std::string, int, std::string, double> QueryData;
				typedef std::vector<QueryData> QueryDataList;
				QueryDataList list;

				Statement select(sess);
				select << FIRST_RULE,
					into(list),
					now;

				Poco::FileStream str("FraudRule1.txt", std::ios::trunc);
				std::cout << "Fraud Rule 1: \n";
				auto header = std::format("{:<20}{:<25}{:<25}{:<25}{:<25}\n", "Name", "Account Number", "Transaction Number", "Merchant", "Transaction Amount");

				std::cout << header;
				str << header;

				for (QueryDataList::const_iterator it = list.begin(); it != list.end(); ++it)
				{
					auto temp = std::format("{:<20}{:<25}{:<25}{:<25}{:<25}\n", it->get<0>(), it->get<1>(), it->get<2>(), it->get<3>().substr(0, 6), it->get<4>());
					std::cout << temp;
					str << temp;
				}

				str.close();
			}
			// second rule
			{
				typedef Poco::Tuple<std::string, std::string, int, std::string, std::string> QueryData;
				typedef std::vector<QueryData> QueryDataList;
				QueryDataList list;

				Statement select(sess);
				select << SECOND_RULE,
					into(list),
					now;

				std::cout << "\n\nFraud Rule 2: \n";
				Poco::FileStream str("FraudRule2.txt", std::ios::trunc);
				auto header = std::format("{:<20}{:<25}{:<25}{:<35}{:<35}\n", "Name", "Account Number", "Transaction Number", "Expected Transaction Location", "Actual Transaction Location");

				std::cout << header;
				str << header;

				for (QueryDataList::const_iterator it = list.begin(); it != list.end(); ++it)
				{
					auto temp = std::format("{:<20}{:<25}{:<25}{:<35}{:<35}\n", it->get<0>(), it->get<1>(), it->get<2>(), it->get<3>(), it->get<4>());
					std::cout << temp;
					str << temp;
				}

				str.close();
			}

		}
		else
		{
			std::cout << "* * Connected to DB" << "Failed" << std::endl;
		}

		Poco::Data::MySQL::Connector::unregisterConnector();
	}
	catch (Poco::Exception& ex)
	{
		std::cerr << ex.displayText() << std::endl;
	}

	return 0;
}
