#pragma once
/////////////////////////////////////////////////////////////////////
// Browse.h - Implements Browse data in the nosql dababase funcion  //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* provide view the database content of selected item
* provide modify dependency in the database function
* provide find the file has no parent
* 
* Provided Funcions:
* ------------------
* view()               view the database information of the selected file
* changedepend()       verify the dependency change, and modify the database data
* noparent()           return all the file in any category has no parent
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
#include "../../NoSqlDb/Query/Query.h"
#include "../Version/Version.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
using namespace NoSqlDb;
namespace Repository {
	class Browse {
	public:
		DbElement<PayLoad> view(const std::string& file, DbCore<PayLoad>& db);
		bool changedepend(const std::string&key, DbCore<PayLoad>&db,const std::vector<std::string>& childlist);
		std::string noparent(DbCore<PayLoad>&db);
	private:
		Version * verctrl;
	};

	//----< view all the content in database of the selected item>----------------------
	DbElement<PayLoad> Browse::view(const std::string& filename, DbCore<PayLoad>& db) {
		DbElement<PayLoad> ele = db[filename];
		if (ele.children().size() > 0) {
			std::unordered_map<std::string, std::vector<std::string>> deplist;
			for (auto item = db.begin(); item != db.end(); item++) {
				std::string file = item->second.name();
				if (item->second.children().size() > 0 && deplist.find(file) == deplist.end()) deplist[file] = item->second.children();
			}
			std::vector<std::string>output;
			output.push_back(ele.name());
			verctrl->depcheckhelper(ele.name(), deplist, output);
			output.erase(output.begin());
			ele.children().clear();
			for (auto item : output) {
				auto it = std::find(ele.children().begin(), ele.children().end(), item);
				if (it == ele.children().end())ele.addChildKey(item);
			}
		}
		return ele;
	}

	//----<modify the dependency in the database>----------------------
	bool Browse::changedepend(const std::string&key, DbCore<PayLoad>&db, const std::vector<std::string>& childlist) {
		std::string name = verctrl->dever(key);
		if (key != verctrl->findlatest(name,db))return false;
		DbElement<PayLoad> tocheck = db[key];
		db[key].clearChildKeys();
		for (auto item : childlist) {
			db[key].addChildKey(item);
		}
		if (!verctrl->checkDepend(db, db[key])) {
			db[key] = tocheck;
			return false;
		}
		if (verctrl->checkDepend(db, db[key]) == 1)db[key].payLoad().isOpen(true);
		
		return true;
	}

	//----< find which file in the repository has no parent >----------------------
	std::string Browse::noparent(DbCore<PayLoad>&db) {
		std::vector<std::string> namelist;
		std::unordered_map<std::string, std::vector<std::string>> deplist;
		for (auto item = db.begin(); item != db.end(); item++) {
			std::string file = item->second.name();
			auto it = std::find(namelist.begin(), namelist.end(), file);
			if (it == namelist.end())namelist.push_back(file);
			if (item->second.children().size() > 0 && deplist.find(file) == deplist.end()) deplist[file] = item->second.children();
		}
		for (auto item = deplist.begin(); item != deplist.end(); ++item) {
			std::vector<std::string> dep = item->second;
			for (auto it : dep) {
				auto tofind = std::find(namelist.begin(), namelist.end(), it);
				if (tofind != namelist.end())namelist.erase(tofind);
			}
		}
		std::vector<std::string> res = namelist;
		std::string result;
		for (std::size_t i = 0; i < res.size(); ++i) {
			if (i == res.size() - 1) {
				result = result + res[i] + " ";
			}
			else {
				result += res[i];
			}
		}
		return result;
	}
}
