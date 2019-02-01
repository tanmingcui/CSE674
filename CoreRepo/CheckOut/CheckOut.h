#pragma once
/////////////////////////////////////////////////////////////////////
// CheckOut.h - Implements Repository Check Out funcion            //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* Provide check out function. It will check out the selected files and
* all the dependency files to the client
*
* Provided Funcions:
* ------------------
* tocheckout()                checkout the item in db according to the name 
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* PayLoad.h, PayLoad.cpp
* FileSystem.h, FileSystem.cpp
* Version.h, Version.cpp
*
* Maintenance History:
* --------------------
* ver 1.0 : Apirl 24 2018
* - first release
*/
#include "../../NoSqlDb/DateTime/DateTime.h"
#include "../../NoSqlDb/DbCore/DbCore.h"
#include "../../NoSqlDb/PayLoad/PayLoad.h"
#include "../../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../Version/Version.h"
#include <string>
#include <iostream>
using namespace NoSqlDb;
namespace Repository {
	class CheckOut {
	public:
		DbElement<PayLoad> tocheckout(const std::string& filename, DbCore<PayLoad>& db);
		void CheckOutsetOriginpath(const std::string& originpath) { _originpath = originpath; }
		void CheckOutsetDestpath(const std::string& destpath) { _destpath = destpath; }
	private:
		void moveFile(const std::string& from, const std::string& to);
		std::string _originpath = "../Repository";
		std::string _destpath = "../ServerFile";
		Version* verctrl;
	};

	//----< main check out function >----------------------
	DbElement<PayLoad> CheckOut::tocheckout(const std::string& filename, DbCore<PayLoad>& db) {
		DbElement<PayLoad> res;
		if (db.contains(filename)) {
			DbElement<PayLoad> ele = db[filename];
			res = ele;
			std::string frompath = ele.payLoad().value();
			std::string fileName = ele.name();
			std::string topath = FileSystem::Path::fileSpec(FileSystem::Path::getFullFileSpec(_destpath), fileName);
			moveFile(frompath, topath);

			if (ele.children().size() > 0) {
				//get all the child
				
				std::unordered_map<std::string, std::vector<std::string>> deplist;
				for (auto item = db.begin(); item != db.end(); item++) {
					std::string file = item->second.name();
					if (item->second.children().size() > 0 && deplist.find(file) == deplist.end()) deplist[file] = item->second.children();
				}
				std::vector<std::string>output;
				output.push_back(verctrl->dever(filename));
				verctrl->depcheckhelper(verctrl->dever(filename), deplist, output);
				output.erase(output.begin());
				res.clearChildKeys();
				for (auto item : output) {
					if (verctrl->isExist(item, db)) {
						res.addChildKey(item);
						DbElement<PayLoad> child = db[verctrl->findlatest(item, db)];
						frompath = child.payLoad().value();
						fileName = child.name();
						topath = FileSystem::Path::fileSpec(FileSystem::Path::getFullFileSpec(_destpath), fileName);
						moveFile(frompath, topath);
					}
				}
			}
		}
		return res;
	}

	//----< copy file from repository folder to temp >----------------------
	void CheckOut::moveFile(const std::string& from, const std::string& to) {
		FileSystem::File me(from);
		me.open(FileSystem::File::in, FileSystem::File::binary);
		FileSystem::File you(to);
		you.open(FileSystem::File::out, FileSystem::File::binary);
		while (me.isGood() && you.isGood())
		{
			static size_t count = 0;
			FileSystem::Block b = me.getBlock(1024);
			you.putBlock(b);
		}
		FileSystem::File::remove(from);
		return;
	}
}