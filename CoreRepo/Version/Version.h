#pragma once
/////////////////////////////////////////////////////////////////////
// Version.h - Basic versioning function                           //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* append version number to the checkin file.
* tear off version number of the file in the repository
* find the latest version of the file in the repository
* helper function for checkin checkout and browse
*
* Provided Funcions:
* ------------------
* ver()                 append the version number to the filename
* findlatest()          find the latest version of the file in the repository
* dever()               tear off the version number of the file
* isExist()             find if the file exist in the repository
* depcheckhelper()      recursive check dependency helper function 
* verNum()              return the version number of the filename
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* PayLoad.h, PayLoad.cpp
* FileSystem.h, FileSystem.cpp
*
* Maintenance History:
* --------------------
* ver 1.0 : Apirl 24 2018
* - first release
*/
#include "../../NoSqlDb/DbCore/DbCore.h"
#include "../../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../../NoSqlDb/PayLoad/PayLoad.h"
#include "../../NoSqlDb/Query/Query.h"
#include <string>
#include <vector>
using namespace NoSqlDb;
namespace Repository {
	class Version {
	public:
		std::string ver(const std::string& filename,DbCore<PayLoad>& db);
		std::string findlatest(const std::string& filename, DbCore<PayLoad>& db);
		std::string dever(const std::string& filename);
		bool isExist(const std::string& filename, DbCore<PayLoad>& db);
		void depcheckhelper(const std::string& filename, std::unordered_map<std::string, std::vector<std::string>>& deplist, std::vector<std::string>& output);
		int verNum(const std::string& filename);
		int checkDepend(DbCore<PayLoad>& db, DbElement<PayLoad>& fileinfo);
	};

	//----< tear off the version number of the filename >----------------------
	std::string Version::dever(const std::string& filename) {
		std::size_t length = filename.size();
		std::size_t pos = length - 1;
		for (; filename[pos] != 'v'; pos--);
		std::size_t start = 0;
		std::string ver = filename.substr(start, pos-1);
		return ver;
	}

	//----< append the version number of the filename >----------------------
	std::string Version::ver(const std::string& filename,DbCore<PayLoad>& db){
		std::string newName;
		if (isExist(filename,db)) {
			std::string latest = findlatest(filename,db);
			int latestnum = verNum(latest) + 1;
			newName = std::string( filename + ".v" + std::to_string(latestnum));
		}
		else {
			newName = std::string(filename + ".v1");
		}
		return newName;
	}

	
	//----< Check whether the file exist in the database >----------------------
	bool Version::isExist(const std::string& filename, DbCore<PayLoad>& db) {
		for (auto it = db.begin(); it != db.end(); ++it) {
			DbElement<PayLoad> ele = it->second;
			if (ele.name() == filename)return true;
		}
		return false;
	}

	//----< find the latest version of the file in the repository >----------------------
	std::string Version::findlatest(const std::string& filename, DbCore<PayLoad>& db){
		std::string res = " ";
		int tocompare = 0;
		Conditions<PayLoad> con;
		con.name(filename);
		Query<PayLoad> query(db);
		
		std::vector<std::string> qres;
		query.select(con).keys();
		for (auto it = db.begin(); it != db.end(); ++it) {
			DbElement<PayLoad> ele = it->second;
			if (ele.name() == filename)qres.push_back(it->first);
		}
		for (std::string ele : qres) {
			if (verNum(ele) > tocompare)res = ele;
		}
		return res;
	}

	//----< return the version number of the file >----------------------
	int Version::verNum(const std::string& filename){
		std::size_t length = filename.size();
		std::size_t pos = length - 1;
		for (; filename[pos] != 'v'; pos--);
		std::string ver = filename.substr(pos + 1, length - 1);
		int res = std::stoi(ver);
		return res;
	}

	//----< Recursive search helper funstion for dependency check >----------------------
	void Version::depcheckhelper(const std::string& filename, std::unordered_map<std::string, std::vector<std::string>>& deplist, std::vector<std::string >& output) {
		if (deplist.find(filename) == deplist.end()) return;
		std::vector<std::string> childlist = deplist[filename];
		for (auto file : childlist) {
			/*output.push_back(file);*/
			if (file == output[0]) {
				output.clear();
				output.push_back("round");
				break;
			}
			depcheckhelper(file, deplist, output);
			output.push_back(file);
		}

	}

	//----< Dependency Check >----------------------
	//0 indicates round dependency
	//1 indicates dependency files not all exisit or open, open status
	//2 indicates dependency files all exisit and closed, close status
	int Version::checkDepend(DbCore<PayLoad>& db, DbElement<PayLoad>& fileinfo) {
		std::vector<std::string> depends = fileinfo.children();
		std::string filename = fileinfo.name();
		std::unordered_map<std::string, std::vector<std::string>> deplist;
		if (depends.size() == 0) {
			std::string thelatest = findlatest(filename, db);
			if (thelatest == " ") {
				if (fileinfo.payLoad().isOpen()) return 1;
				if (!fileinfo.payLoad().isOpen())return 2;
			}
			if (thelatest != " ") {
				if (db[thelatest].payLoad().isOpen())return 1;
				if (!db[thelatest].payLoad().isOpen())return 2;
			}

		}
		else {
			deplist[filename] = depends;
			for (auto item = db.begin(); item != db.end(); item++) {
				std::string file = item->second.name();
				if (item->second.children().size() > 0 && deplist.find(file) == deplist.end()) deplist[file] = item->second.children();
			}
			std::vector<std::string>output;
			output.push_back(filename);
			depcheckhelper(filename, deplist, output);
			auto it = std::find(output.begin(), output.end(), "round");
			if (it != output.end())return 0;

			for (auto item : output) {
				if (!isExist(item, db))return 1;
				else {
					//check the dependency file is open or not
					DbElement<PayLoad> elem = db[findlatest(item, db)];
					if (elem.payLoad().isOpen()) return 1;
				}
			}
		}
		return 2;
	}
}
