#pragma once

///////////////////////////////////////////////////////////////////////
// ServerPrototype.h - Console App that processes incoming messages  //
// ver 1.0                                                           //
// Tanming Cui, CSE687 - Object Oriented Design, Spring 2018         //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018         //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  Package contains one class, Server, that contains a Message-Passing Communication
*  facility. It processes each message by invoking an installed callable object
*  defined by the message's command key.
*
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
*
*  Required Files:
* -----------------
*  ServerPrototype.h, ServerPrototype.cpp
*  Comm.h, Comm.cpp, IComm.h
*  Message.h, Message.cpp
*  FileSystem.h, FileSystem.cpp
*  Utilities.h
*  CheckIn.h
*  CheckOut.h
*  Version.h
*  Browse.h
*
*  Maintenance History:
* ----------------------
*  ver 1.1 : 04/09/2018
*  Add send/receive file function
* ----------------------
*  ver 1.0 : 3/27/2018
*  - first release
*/
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include "../NoSqlDb/DateTime/DateTime.h"
#include "../CppCommWithFileXfer/Message/Message.h"
#include "../CppCommWithFileXfer/MsgPassingComm/Comm.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../NoSqlDb/DbCore/DbCore.h"
#include "../NoSqlDb/PayLoad/PayLoad.h"
#include "../CoreRepo/CheckIn/CheckIn.h"
#include "../CoreRepo/CheckOut/CheckOut.h"
#include "../CoreRepo/Browse/Browse.h"
#include "../CoreRepo/Version/Version.h"
#include <windows.h>
#include <tchar.h>

namespace Repository
{
	class ServerFunc : public CheckIn, public CheckOut, public Version, public Browse {
	};

	using File = std::string;
	using Files = std::vector<File>;
	using Dir = std::string;
	using Dirs = std::vector<Dir>;
	using SearchPath = std::string;
	using Key = std::string;
	using Msg = MsgPassingCommunication::Message;
	using ServerProc = std::function<Msg(Msg, NoSqlDb::DbCore<NoSqlDb::PayLoad>&,ServerFunc&)>;
	using MsgDispatcher = std::unordered_map<Key, ServerProc>;

	const SearchPath storageRoot = "../Repository";  // root for all server file storage
	const MsgPassingCommunication::EndPoint serverEndPoint("localhost", 8080);  // listening endpoint

	class Server
	{
	public:
		Server(MsgPassingCommunication::EndPoint ep, const std::string& name);
		void start();
		void stop();
		void addMsgProc(Key key, ServerProc proc);
		void processMessages();
		void postMessage(MsgPassingCommunication::Message msg);
		MsgPassingCommunication::Message getMessage();
		static Dirs getDirs(const SearchPath& path = storageRoot);
		static Files getFiles(const SearchPath& path = storageRoot);
		//static std::string serverpath() { return FileSystem::Path::getFullFileSpec("../ServerFile");}
	private:
		MsgPassingCommunication::Comm comm_;
		MsgDispatcher dispatcher_;
		std::thread msgProcThrd_;
		NoSqlDb::DbCore<NoSqlDb::PayLoad> _db;
	};


	

	//----< initialize server endpoint and give server a name >----------

	inline Server::Server(MsgPassingCommunication::EndPoint ep, const std::string& name)
		: comm_(ep, name) 
	{
	}

	//----< start server's instance of Comm >----------------------------

	inline void Server::start()
	{
		comm_.start();
	}
	//----< stop Comm instance >-----------------------------------------

	inline void Server::stop()
	{
		if (msgProcThrd_.joinable())
			msgProcThrd_.join();
		comm_.stop();
	}
	//----< pass message to Comm for sending >---------------------------

	inline void Server::postMessage(MsgPassingCommunication::Message msg)
	{
		comm_.postMessage(msg);
	}
	//----< get message from Comm >--------------------------------------

	inline MsgPassingCommunication::Message Server::getMessage()
	{
		Msg msg = comm_.getMessage();
		return msg;
	}
	//----< add ServerProc callable object to server's dispatcher >------

	inline void Server::addMsgProc(Key key, ServerProc proc)
	{
		dispatcher_[key] = proc;
	}
	//----< start processing messages on child thread >------------------
	inline void Server::processMessages()
	{
		auto proc = [&]()
		{
			if (dispatcher_.size() == 0)
			{
				std::cout << "\n  no server procs to call";
				return;
			}
			while (true)
			{
				try {
					Msg msg = getMessage();
					std::cout << "\n  received message: " << msg.command() << " from " << msg.from().toString();
					if (msg.containsKey("verbose"))
					{
						std::cout << "\n";
						msg.show();
					}
					if (msg.command() == "serverQuit")
						break;
					ServerFunc func;
					Msg reply = dispatcher_[msg.command()](msg, _db, func);
					if (msg.to().port != msg.from().port)  // avoid infinite message loop
					{
						postMessage(reply);
						msg.show();
						reply.show();
					}
					else
						std::cout << "\n  server attempting to post to self";
				}
				catch (std::exception& ex) {
					std::cout << "\n" << ex.what();
				}
			}
			std::cout << "\n  server message processing thread is shutting down";
		};
		std::thread t(proc);
		//SetThreadPriority(t.native_handle(), THREAD_PRIORITY_HIGHEST);
		std::cout << "\n  starting server thread to process messages";
		msgProcThrd_ = std::move(t);
	}
}
