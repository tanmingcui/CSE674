/////////////////////////////////////////////////////////////////////
// CheckOut.cpp - Check Out funcion test                           //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
#include "../../NoSqlDb/DateTime/DateTime.h"
#include "../../NoSqlDb/DbCore/DbCore.h"
#include "../../NoSqlDb/PayLoad/PayLoad.h"
#include "../../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../Version/Version.h"
#include "../CheckIn/CheckIn.h"
#include "CheckOut.h"
#include "../CheckIn/CheckIn.h"
#include <string>

using namespace Repository;

#ifdef TEST_CHECKOUT
int main()
{
	//mock check in and then check out
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
	tocheck.show(db);
	CheckOut checkout;
	NoSqlDb::showElem(checkout.tocheckout("test2.txt.v1", db));
	return 0;
}
#endif // TEST_CHECKOUT




