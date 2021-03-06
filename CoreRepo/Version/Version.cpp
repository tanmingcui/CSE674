/////////////////////////////////////////////////////////////////////
// Version.cop - Test case for all version function                //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////

#include "Version.h"
#include "../../NoSqlDb/DbCore/DbCore.h"
#include "../../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../../NoSqlDb/PayLoad/PayLoad.h"
#include "../../NoSqlDb/Query/Query.h"
#include <string>
#include <vector>
using namespace Repository;

#ifdef TEST_VERSION
int main()
{
	Version ver;
	DbCore<PayLoad> db;
	DbElement<PayLoad> ele;
	ele.name("a.h");
	db.addRecord("a.h.v1", ele);
	db.addRecord("a.h.v2", ele);
	db.addRecord("a.h.v3", ele);
	std::cout << ver.ver("a.h", db) << "\n";
	std::cout << ver.ver("b.h", db) << "\n";
	std::cout << ver.findlatest("a.h", db) << "\n";
	std::cout << ver.dever("a.h.v10") << "\n";
	return 0;
}

#endif // TEST_VERSION


