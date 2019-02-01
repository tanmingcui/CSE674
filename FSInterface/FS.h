#pragma once
/////////////////////////////////////////////////////////////////////
// FS.h - File System for C# GUI                                   //
// ver 1.0                                                         //
// Tanming Cui, CSE687 Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  - Read/Write a file from/to a specific location
*  - Basic Path function provided in file system package 
*
*  Required Files:
*  ---------------
*  FS.h, Ifs.h, FileSystem.h
*  FileSystem.cpp
*
*  Maintenance History:
*  --------------------
*  ver 1.0 : 10 Apr 2018
*  - first realease
*
*/
#include "Ifs.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <string>
#include <vector>
namespace MsgPassingCommunication {

	class FS :public Ifs {
	public:
		FS() {}
		std::vector<std::string> getdirs(const std::string& path);
		std::vector<std::string> getfiles(const std::string& path);
		std::vector<std::string> read(const std::string& path);
		void write(const std::vector<std::string>& content, const std::string& outputpath);
		std::string getname(const std::string& path);
		std::string getpath(const std::string& path);
		std::string filespec(const std::string& path, const std::string& name);
	};

	inline Ifs* Ifs::createFS()
	{
		Ifs* pfs = new FS();
		return pfs;
	}
	
}