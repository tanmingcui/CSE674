/////////////////////////////////////////////////////////////////////
// FS.cpp - File System for C# WPF                                 //
// ver 1.0                                                         //
// Tanming Cui, CSE687 Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "FS.h"
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>
#include <utility>
#include <clocale>
#include <locale>

using namespace FileSystem;
using namespace MsgPassingCommunication;

//----< get directories >--------------------------------------
std::vector<std::string> FS::getdirs(const std::string& path) {
	std::string fullpath = Path::getFullFileSpec(path);
	return Directory::getDirectories(fullpath);
}

//----< get files >--------------------------------------
std::vector<std::string> FS::getfiles(const std::string& path) {
	std::string fullpath = Path::getFullFileSpec(path);
	return Directory::getFiles(fullpath);
}

//----< read file content >--------------------------------------
std::vector<std::string> FS::read(const std::string &path) {
	std::vector<std::string>res;
	File file(path);
	file.open(File::in);
	
	while (true)
	{
		std::string store;
		if (!file.isGood())break;
		store = file.getLine(false);
		std::locale loc;
		if (store.size() > 0 && !std::isspace(store[store.size() - 1], loc))store += ' ';
		res.push_back(store);
	}
	return res;
}

//----< write to a file  >--------------------------------------
void FS::write(const std::vector<std::string>& content, const std::string& outputpath) {
	File out(outputpath);
	out.open(File::out, File::text);
	for (auto line : content) out.putLine(line);
}

//----< combine path and file name >--------------------------------------
std::string FS::filespec(const std::string& path, const std::string& name) {
	return Path::fileSpec(path, name);
}

//----< get file name in the path >--------------------------------------
std::string FS::getname(const std::string& path) {
	return Path::getName(path);
}

//----< get file path in the full path >--------------------------------------
std::string FS::getpath(const std::string& path) {
	return Path::getName(path);
}

#ifdef TEST_FS
int main() {
	FS fs = FS();
	for (auto item : fs.getfiles("C:")) {
		std::cout << item + "\n";
		
	}

	for (auto item : fs.getdirs("C:")) {
		std::cout << item + "\n";
		std::cout << fs.read(item);
		std::cout << fs.getname(item);
		std::cout << fs.getpath(item);

	}

}
#endif