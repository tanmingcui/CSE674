#pragma once
/////////////////////////////////////////////////////////////////////
// CheckIn.h - Implements Repository Check In funcion              //
// ver 1.0                                                         //
// Tanming Cui CSE687 - Object Oriented Design, Spring 2018        //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* It provides check-in class, move file from temperary folder to repository
* folder, determin whether the file is open or closed according to the 
* dependency file. Check if the file has the round dependency, reject the 
* check in request if it has a round dependency. Append the version to the
* file, update the data in the NoSQL database
*
* Provided Funcions:
* ------------------
* SetOriginPath()              Set the temperary folder path
* SetDestPath()                Set the repository folder
* tocheckin()                  Provide checkin funcion
* getInfo()                    Extract information from xml file
* moveFile()                   Move file from temperary folder to repository folder
* show()                       Show the content of the nosql database
* getPath()                    Full file path in the repository
* originPath()                 Full file path in the temperary folder
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* PayLoad.h, PayLoad.cpp
* FileSystem.h, FileSystem.cpp
* Version.h, Version.cpp
* Persist.h, Persist.cpp
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
#include "../../NoSqlDb/Persist/Persist.h"
#include "../Version/Version.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace NoSqlDb;

namespace Repository {
	class CheckIn {
	public:
		void CheckInsetOriginpath(const std::string& originpath) { _originpath = originpath; }
		void CheckInsetDestpath(const std::string& destpath) { _destpath = destpath; }
		bool tocheckin(DbElement<PayLoad>& fileinfo, DbCore<PayLoad>& db);
		DbCore<PayLoad> getInfo(const std::string& xmlpath);
		void show(DbCore<PayLoad>& db);
	private:
		std::string _originpath = "../ServerFile";     //default temperaray path
		std::string _destpath = "../Repository";	   //default repository path
		void moveFile(const std::string& from, const std::string& to);
		std::string getPath(const std::vector<std::string> cata, const std::string& filename);
		std::string originPath(const std::string& filename);
		Version* verctrl;
	};
	
	//----< Return the full path of the raw file in the temp folder >----------------------
	std::string CheckIn::originPath(const std::string& filename) {
		std::string originfull = FileSystem::Path::getFullFileSpec(_originpath);
		originfull = FileSystem::Path::fileSpec(originfull, filename);
		return originfull;
	}

	//----< Return the full path of the file in destination folder >----------------------
	std::string CheckIn::getPath(const std::vector<std::string> cata, const std::string& filename) {
		std::string fullpath(FileSystem::Path::getFullFileSpec(_destpath));
		for (auto dir : cata) fullpath = FileSystem::Path::fileSpec(fullpath, dir);
		fullpath = FileSystem::Path::fileSpec(fullpath, filename);
		return fullpath;
	}

	//----< get information from xml files, and write it to database >----------------------
	DbCore<PayLoad> CheckIn::getInfo(const std::string& xmlpath) {
		DbCore<PayLoad> db;
		FileSystem::File xmlfile(xmlpath);
		xmlfile.open(FileSystem::File::in);
		std::string content = xmlfile.readAll();
		Persist<PayLoad> per(db);
		per.fromXml(content, rebuild);
		return db;
	}
	void CheckIn::show(DbCore<PayLoad>& db) {
		showDb(db);
	}


	//----< main check in funcion>----------------------
	bool CheckIn::tocheckin(DbElement<PayLoad>& fileinfo, DbCore<PayLoad>& db) {
		std::string filename = fileinfo.name();
		std::string fullorigin = originPath(fileinfo.name());
		if (!verctrl->checkDepend(db, fileinfo)) {
			FileSystem::File::remove(fullorigin);
			return false;
		}
		else if (verctrl->isExist(filename, db)) {
			if (verctrl->checkDepend(db, fileinfo) == 2) {
				std::string newname = verctrl->ver(filename, db);
				std::string fulldest = getPath(fileinfo.payLoad().categories(), newname);
				fileinfo.payLoad().value(fulldest);
				moveFile(fullorigin, fulldest);
				db.addRecord(newname, fileinfo);
			}
			else {
				std::string thename = verctrl->findlatest(filename, db);
				std::string fullpath = getPath(fileinfo.payLoad().categories(), thename);
				fileinfo.payLoad().value(fullpath);
				if (FileSystem::File::exists(fullpath))FileSystem::File::remove(fullpath);
				db.removeRecord(thename);
				moveFile(fullorigin, fullpath);
				fileinfo.payLoad().isOpen(true);
				db.addRecord(thename, fileinfo);
			}
		}
		else {
			std::vector<std::string> dep = fileinfo.children();
			std::vector<std::string> dirs = fileinfo.payLoad().categories();
			std::string newName = verctrl->ver(filename, db);
			std::string destpath = getPath(dirs, newName);
			std::string dirpath = FileSystem::Path::getPath(destpath);
			if (!FileSystem::Directory::exists(dirpath)) {
				std::string dirtocreate = FileSystem::Path::getFullFileSpec(_destpath);
				for (auto dir : dirs) {
					dirtocreate = FileSystem::Path::fileSpec(dirtocreate, dir);
					FileSystem::Directory::create(dirtocreate);
				}
			}
			if (!verctrl->checkDepend(db, fileinfo))return false;
			if (verctrl->checkDepend(db, fileinfo) == 1) fileinfo.payLoad().isOpen(true);
			fileinfo.payLoad().value(destpath);
			moveFile(fullorigin, destpath);
			db.addRecord(newName, fileinfo);
		}
		FileSystem::File::remove(fullorigin);
		return true;
	}

	//----< copy file from temperary folder to repository >----------------------
	void CheckIn::moveFile(const std::string& from, const std::string& to) {
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
	}	
}
