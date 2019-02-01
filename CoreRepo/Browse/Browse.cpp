/////////////////////////////////////////////////////////////////////
// Browse.cpp - Test all Browse funcions                           //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
#include "../../NoSqlDb/DateTime/DateTime.h"
#include "../../NoSqlDb/DbCore/DbCore.h"
#include "../../NoSqlDb/PayLoad/PayLoad.h"
#include "../../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../Version/Version.h"
#include "../CheckIn/CheckIn.h"
#include "Browse.h"
#include <string>

#ifdef TEST_BROWSE



using namespace Repository;
int main() {
	CheckIn tocheck;
	DbCore<PayLoad> db;
	DbElement<PayLoad> ele;
	ele.name("test.txt");
	ele.payLoad().categories().push_back("b");
	ele.payLoad().categories().push_back("c");
	tocheck.tocheckin(ele, db);
	DbElement<PayLoad> ele1;
	ele1.name("test2.txt");
	ele1.payLoad().categories().push_back("b");
	ele1.payLoad().categories().push_back("c");
	ele1.addChildKey("test.txt");
	tocheck.tocheckin(ele1, db);
	DbElement<PayLoad> ele0;
	ele0.name("test1.txt");
	ele0.payLoad().categories().push_back("x");
	ele0.payLoad().categories().push_back("y");
	ele0.addChildKey("test.txt");
	ele0.addChildKey("test2.txt");
	tocheck.tocheckin(ele0, db);
	DbElement<PayLoad> ele3;
	ele3.name("test3.txt");
	ele3.payLoad().categories().push_back("d");
	ele3.payLoad().categories().push_back("e");
	ele3.addChildKey("test4.txt");
	tocheck.tocheckin(ele3, db);
	Browse View;
	NoSqlDb::showElem(View.view("test1.txt.v1",db));
	return 0;
}

#endif // TEST_BROWSE