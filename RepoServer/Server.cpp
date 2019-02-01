/////////////////////////////////////////////////////////////////////////
// Server.cpp - Console App that processes incoming messages           //
// ver 1.0                                                             //
// Tanming Cui, CSE687 - Object Oriented Design, Spring 2018           //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018           //
/////////////////////////////////////////////////////////////////////////

#include "ServerPrototype.h"
#include <sstream>
#include <iomanip>
#include <utility>
#include <clocale>
#include <locale>
#include "../NoSqlDb/DateTime/DateTime.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../CoreRepo/CheckIn/CheckIn.h"
#include "../CoreRepo/CheckOut/CheckOut.h"
#include "../CoreRepo/Browse/Browse.h"
#include "../CoreRepo/Version/Version.h"
#include "../NoSqlDb/DbCore/DbCore.h"
#include "../NoSqlDb/PayLoad/PayLoad.h"
#include <unordered_map>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;

//----< reply get files helper function>----------
Files Server::getFiles(const Repository::SearchPath& path)
{
	return Directory::getFiles(path);
}

//----< reply get directories helper function >----------
Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}

//----< show message content >----------
template<typename T>
void show(const T& t, const std::string& msg)
{
	std::cout << "\n  " << msg.c_str();
	for (auto item : t)
	{
		std::cout << "\n    " << item.c_str();
	}
}


//----< reply get files >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> getFiles = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getFiles");
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files files = Server::getFiles(searchPath);
		size_t count = 0;
		for (auto item : files)
		{
			std::string countStr = Utilities::Converter<size_t>::toString(++count);
			reply.attribute("file" + countStr, item);
		}
	}
	else
	{
		std::cout << "\n  getFiles message did not define a path attribute";
	}
	return reply;
};

//----< reply client modify request>----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replymodify = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&db, ServerFunc&func) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::stringstream dep(msg.value("dependency"));
	std::string dependencies;
	std::vector<std::string> deplist;
	while (getline(dep, dependencies, ' ')) deplist.push_back(dependencies);
	if (func.changedepend(msg.value("key"), db, deplist))
	{
		func.show(db);
		reply.command("modifysuccess");
	}
	else {
		reply.command("modifyfail");
	}
	return reply;
};

//----< reply client check file has no parent>----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replynoparent = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&db, ServerFunc&func) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string nopar = func.noparent(db);
	reply.command("noparent");
	reply.attribute("are", nopar);
	return reply;
};

//----< reply get connect request >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replyconnect = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("connected");
	return reply;
};

//----< reply check in request >----------
std::function<Msg(Msg,NoSqlDb::DbCore<NoSqlDb::PayLoad>&,ServerFunc&)> replycheckin = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>& db, ServerFunc& func) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	DbElement<PayLoad> info;
	info.name(FileSystem::Path::getName(msg.value("sendingFile")));
	DateTime now;
	std::string isOpen = msg.value("isOpen");
	if (isOpen == "open") info.payLoad().isOpen(true);
	if (isOpen == "close")info.payLoad().isOpen(false);
	info.descrip(msg.value("description"));
	info.dateTime(now.now());
	std::string pattoken;
	std::string dependencies;
	std::stringstream inpat(msg.value("category"));
	std::stringstream dep(msg.value("dependency"));
	while (getline(inpat, pattoken, ' ')) info.payLoad().categories().push_back(pattoken);
	while (getline(dep, dependencies, ' '))info.addChildKey(dependencies);
	if(func.tocheckin(info, db))reply.command("finishcheckin");
	else reply.command("rounddependency");
	NoSqlDb::showDb(db);
	return reply;
};

//----< reply show full text request >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> filefulltext = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string path = msg.value("path");
	path = Path::getFullFileSpec(path);
	reply.command("filefulltext");
	reply.attribute("sendingFile", path);
	reply.attribute("destination", FileSystem::Path::getFullFileSpec("../ClientFile"));
	reply.attribute("verbose", "haha");
	return reply;
};

//----< when checkout, continue to send the dependency files>----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replygetChilds = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>& db, ServerFunc& func) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string filename = msg.value("childname");
	if (func.isExist(filename,db)) {
		reply.command("childfiles");
		reply.attribute("sendingFile", FileSystem::Path::fileSpec(FileSystem::Path::getFullFileSpec("../ServerFile"), filename));
		reply.attribute("destination", FileSystem::Path::getFullFileSpec("../ClientFile"));
		reply.attribute("verbose", "haha");
	}
	return reply;
};

//----< reply check out request >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replycheckout = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>& db, ServerFunc& func) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string key = msg.value("key");
	DbElement<PayLoad>res=func.tocheckout(key, db);
	reply.attribute("sendingFile", FileSystem::Path::fileSpec(FileSystem::Path::getFullFileSpec("../ServerFile"), res.name()));
	reply.attribute("destination", FileSystem::Path::getFullFileSpec("../ClientFile"));
	reply.attribute("verbose", "haha");
	if (res.children().size() == 0)reply.command("finishcheckout");
	else {
		reply.command("haschildren");
		int count = 0;
		for (auto item : res.children()) {
			count++;
			reply.attribute("child" + std::to_string(count), item);
		}
	}
	return reply;
};

//----< reply send file request >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replysendfile = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("received");
	return reply;
};



//----< reply view metadata request >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> replyviewmeta = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>& db, ServerFunc& func) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string key = msg.value("key");
	NoSqlDb::DbElement<NoSqlDb::PayLoad> res = func.view(key, db);
	int version = func.verNum(key);
	std::vector<std::string>depend = res.children();
	std::vector<std::string> cate = res.payLoad().categories();
	std::string dependencies;
	std::string category;
	for (std::size_t i = 0; i < cate.size(); ++i) {
		if (i == cate.size() - 1)category += cate[i];
		else category = category + cate[i] + " ";
	}
	for (std::size_t i = 0; i < depend.size(); ++i) {
		if (i == depend.size() - 1)dependencies += depend[i];
		else dependencies = dependencies + depend[i] + " ";
	}
	reply.attribute("name", res.name());
	reply.attribute("description", res.descrip());
	reply.attribute("datetime", std::string(res.dateTime()));
	reply.attribute("dependency", dependencies);
	reply.attribute("location", res.payLoad().value());
	reply.attribute("category", category);
	reply.attribute("version", std::to_string(version));
	if (res.payLoad().isOpen())reply.attribute("isOpen", "open");
	if(!res.payLoad().isOpen())reply.attribute("isOpen","close");

	reply.command("metadata");
	return reply;
};

//----< reply gedirs request >----------
std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&)> getDirs = [](Msg msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&, ServerFunc&) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getDirs");
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files dirs = Server::getDirs(searchPath);
		size_t count = 0;
		for (auto item : dirs)
		{
			if (item != ".." && item != ".")
			{
				std::string countStr = Utilities::Converter<size_t>::toString(++count);
				reply.attribute("dir" + countStr, item);
			}
		}
	}
	else
	{
		std::cout << "\n  getDirs message did not define a path attribute";
	}
	return reply;
};


//----< server main function >----------
int main()
{
	std::cout << "\n  Testing Server Prototype";
	std::cout << "\n ==========================";
	std::cout << "\n";
	//StaticLogger<1>::attach(&std::cout);
	//StaticLogger<1>::start();
	Server server(serverEndPoint, "ServerPrototype");
	server.start();
	server.addMsgProc("connect",replyconnect);
	server.addMsgProc("sendfile", replysendfile);
	server.addMsgProc("getFiles", getFiles);
	server.addMsgProc("getDirs", getDirs);
	server.addMsgProc("checkin", replycheckin);
	server.addMsgProc("checkout", replycheckout);
	server.addMsgProc("getfilefulltext", filefulltext);
	server.addMsgProc("viewmetadata", replyviewmeta);
	server.addMsgProc("noparent", replynoparent);
	server.addMsgProc("modify", replymodify);
	server.addMsgProc("getChilds", replygetChilds);
	server.processMessages();
	std::cout << "\n  press enter to exit";
	std::cin.get();
	std::cout << "\n";
	server.stop();
	return 0;
}

