#pragma once
/////////////////////////////////////////////////////////////////////
// Ifs.h - interface for File System management facility           //
// ver 1.0                                                         //
// Tanming Cui, CSE687 Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>

namespace MsgPassingCommunication {
	class Ifs {
	public:
		static Ifs* createFS();
		virtual std::vector<std::string> getdirs(const std::string& path) = 0;
		virtual std::vector<std::string> getfiles(const std::string& path) = 0;
		virtual std::vector<std::string> read(const std::string& path) = 0;
		virtual std::string filespec(const std::string& path, const std::string& name) = 0;
		virtual std::string getname(const std::string& path) = 0;
		virtual std::string getpath(const std::string& path) = 0;
		virtual void write(const std::vector<std::string>& content, const std::string& outputpath) = 0;
		virtual ~Ifs() {}
	};

}
