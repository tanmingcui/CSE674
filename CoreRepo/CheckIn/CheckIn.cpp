/////////////////////////////////////////////////////////////////////
// CheckIn.cpp - Test case for all Check In funcions               //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
#include "CheckIn.h"
#include "../../NoSqlDb/DateTime/DateTime.h"
#include "../../NoSqlDb/DbCore/DbCore.h"
#include "../../NoSqlDb/PayLoad/PayLoad.h"
#include "../../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../../NoSqlDb/Persist/Persist.h"
#include "../Version/Version.h"
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

/*NoSqlDb::DbCore<NoSqlDb::PayLoad> res =tocheck.getInfo("testcheckin.xml");
	tocheck.show(res);*/

using namespace Repository;

#ifdef TEST_CHECKIN
int main() {
	CheckIn tocheck;
	DbCore<PayLoad> db;
	DbElement<PayLoad> ele;
	ele.name("test.txt");
	ele.payLoad().categories().push_back("b");
	ele.payLoad().categories().push_back("c");
	tocheck.tocheckin(ele,db);
	tocheck.show(db);
	tocheck.tocheckin(ele, db);
	tocheck.show(db);
	ele.payLoad().isOpen(false);
	tocheck.tocheckin(ele, db);
	tocheck.show(db);
	DbElement<PayLoad> ele1;
	ele1.name("test2.txt");
	ele1.payLoad().categories().push_back("b");
	ele1.payLoad().categories().push_back("c");
	ele1.addChildKey("test.txt");
	tocheck.tocheckin(ele1, db);
	tocheck.show(db);
	tocheck.tocheckin(ele1, db);
	tocheck.show(db);
	DbElement<PayLoad> ele0;
	ele3.name("test3.txt");
	ele3.payLoad().categories().push_back("d");
	ele3.payLoad().categories().push_back("e");
	ele3.addChildKey("test4.txt");
	tocheck.tocheckin(ele3, db);
	tocheck.show(db);
	DbElement<PayLoad> ele4;
	ele4.name("test4.txt");
	ele4.payLoad().categories().push_back("d");
	ele4.payLoad().categories().push_back("e");
	ele4.addChildKey("test3.txt");
	if (tocheck.tocheckin(ele4, db)) tocheck.show(db);
	else std::cout << "\n\n round dependency \n\n";
	tocheck.show(db);
}
#endif // TEST_CHECKIN
