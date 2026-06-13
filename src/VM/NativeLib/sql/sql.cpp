#include "sql.h"
#include <mysql/mysql.h>

Value NativeDBConnect(const std::vector<Value>& args)
{
	const auto host = AsString(args[0]);
	const auto user = AsString(args[1]);
	const auto pass = AsString(args[2]);
	const auto dbName = AsString(args[3]);

	auto conn = mysql_init(nullptr);
	if (conn == nullptr)
	{
		throw std::runtime_error("MySQL initialization failed!");
	}
	if (mysql_real_connect(
			conn,
			host.c_str(),
			user.c_str(),
			pass.c_str(),
			dbName.c_str(),
			0,
			nullptr,
			0)
		== nullptr)
	{
		const std::string errorMsg = mysql_error(conn);
		mysql_close(conn);
		throw std::runtime_error("DB Connection failed: " + errorMsg);
	}
	return static_cast<int64_t>(reinterpret_cast<uintptr_t>(conn));
}

Value NativeDBExecute(const std::vector<Value>& args)
{
	const auto conn = reinterpret_cast<MYSQL*>(static_cast<uintptr_t>(AsInt(args[0])));
	const auto query = AsString(args[1]);
	if (mysql_query(conn, query.c_str()))
	{
		throw std::runtime_error("DB Query failed: " + std::string(mysql_error(conn)));
	}

	const auto res = mysql_store_result(conn);
	if (res == nullptr)
	{
		return std::make_shared<Array>(0);
	}
	const int numFields = mysql_num_fields(res);
	const int numRows = mysql_num_rows(res);

	auto resultArr = std::make_shared<Array>(numRows);

	MYSQL_ROW row;
	int rowIndex = 0;
	while ((row = mysql_fetch_row(res)))
	{
		auto rowArr = std::make_shared<Array>(numFields);

		for (int i = 0; i < numFields; i++)
		{
			const auto fieldVal = (row[i] ? row[i] : "NULL");
			rowArr->elements[i] = MakeString(fieldVal);
		}
		resultArr->elements[rowIndex++] = rowArr;
	}

	mysql_free_result(res);
	return resultArr;
}

Value NativeDBDisconnect(const std::vector<Value>& args)
{
	const auto conn = reinterpret_cast<MYSQL*>(static_cast<uintptr_t>(AsInt(args[0])));
	mysql_close(conn);
	return Value{};
}

Value NativeSqlEscape(const std::vector<Value>& args)
{
	const auto input = AsString(args[0]);
	std::string output;
	output.reserve(input.length() + 10);

	for (char c : input)
	{
		if (c == '\'')
		{
			output += "''";
		}
		else if (c == '\\')
		{
			output += "\\\\";
		}
		else
		{
			output += c;
		}
	}

	return MakeString(output);
}
