#pragma once
/////////////////////////////////////////////////////////////////////
// DbCore.h - Implements NoSql database prototype                  //
// ver 1.0                                                         //
// Tanming Cui, CSE687 - Object Oriented Design, Spring 2018       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides three classes:
* - DbCore implements core NoSql database operations.
* - DbElement provides the value part of our key-value database.
*   It contains fields for name, description, date, child collection
*   and a payload field of the template type. 
*-  NoSQLDB provides a real nosql database API for developer to create
* and delete the key/value pair, modify the content of the nosql
* database.
* This package provides functions:
* - NoSQLDB<T> add(key,value)         add key/value pair to nosql database
* - NoSQLDB<T> remove(key,value)      remove key/value pair in nosql database
* - NoSQLDB<T> addchild(key,child)	   append child to the specific value
* - NoSQLDB<T> rmchild(key,children)  remove specific child in the specific value
* - NoSQLDB<T> modifydata(key,value,newvalue) modify the specific value if capable
*
* The package also provides functions for displaying:
* - showKeys()            set of all database keys
* - showElem()            database elements
* - showDB()              all records in the database
* - shownosql()           show the content of real nosql database

* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* Utilities.h, Utilities.cpp
*
* Maintenance History:
* --------------------
* ver 1.2 : 05 Feb 2018
* - added add/deletion of the nosaldatabase and other basic database functions
* ver 1.1 : 19 Jan 2018
* - added code to throw exception in index operators if input key is not in database
* ver 1.0 : 10 Jan 2018
* - first release
*/

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "../DateTime/DateTime.h"

namespace NoSqlDb
{
  /////////////////////////////////////////////////////////////////////
  // DbElement class
  // - provides the value part of a NoSql key-value database

  template<typename T>
  class DbElement
  {
  public:
    using Key = std::string;
    using Children = std::vector<Key>;

    // methods to get and set DbElement fields

    std::string& name() { return name_; }
    std::string name() const { return name_; }
    void name(const std::string& name) { name_ = name; }

    std::string& descrip() { return descrip_; }
    std::string descrip() const { return descrip_; }
    void descrip(const std::string& name) { descrip_ = name; }
    
    DateTime& dateTime() { return dateTime_; }
    DateTime dateTime() const { return dateTime_; }
    void dateTime(const DateTime& dateTime) { dateTime_ = dateTime; }

    Children& children() { return children_; }
    Children children() const { return children_; }
    void children(const Children& children) { children_ = children; }

    T& payLoad() { return payLoad_; }
    T payLoad() const { return payLoad_; }
    void payLoad(const T& payLoad) { payLoad_ = payLoad; }

  private:
    std::string name_;
    std::string descrip_;
    DateTime dateTime_;
    Children children_;
    T payLoad_;
  };
  class payload{
	public:
		using key = std::string;
		using keys = std::vector<std::string>;
		void setstr(const key& input) { str_ = input; }
		void setvec(const keys& inputs) { vec_ = inputs; }
		key& getstr() { return str_; }
		key getstr() const { return str_; }
		keys& getvec() { return vec_; }
		keys getvec()const { return vec_; }
	private:
		key str_;
		keys vec_;
	};

  /////////////////////////////////////////////////////////////////////
  // DbCore class
  // - provides core NoSql db operations
  // - does not provide editing, querying, or persistance operations

  template <typename T>
  class DbCore
  {
  public:
    using Key = std::string;
    using Keys = std::vector<Key>;
    using Children = Keys;
    using DbStore = std::unordered_map<Key,DbElement<T>>;
    using iterator = typename DbStore::iterator;

    // methods to access database elements
    Keys keys();
    bool contains(const Key& key);
    size_t size();
	
	//method to remove key/value
	void rmele(const Key& key){ dbStore_.erase(key); }

    void throwOnIndexNotFound(bool doThrow) { doThrow_ = doThrow; }
    DbElement<T>& operator[](const Key& key);
    DbElement<T> operator[](const Key& key) const;
	DbElement<T> &getelement(const Key& key) { return dbStore_[key]; }
	DbElement<T> getelement(const Key& key)const { return dbStore_[key]; }
    typename iterator begin() { return dbStore_.begin(); }
    typename iterator end() { return dbStore_.end(); }
	
    // methods to get and set the private database hash-map storage
    DbStore& dbStore() { return dbStore_; }
    DbStore dbStore() const { return dbStore_; }
    void dbStore(const DbStore& dbStore) { dbStore_ = dbStore; }
  
  private:
    DbStore dbStore_;
    bool doThrow_ = false;
  };
  /////////////////////////////////////////////////////////////////////
  // NoSQL database class
  // - provides the structrue for nosqldb
  template<typename T>
  class NoSQLDB
  {
  public:
	  DbCore<T>& nosqldb() { return nosqldb_; }
	  DbCore<T> nosqldb() const { return nosqldb_; }
	  void nosqldb(const DbCore<T> &nosqldb1) { nosqldb_ = nosqldb1; }

	  //method to add key/value pair
	  NoSQLDB<T>& add(const std::string& key, const DbElement<T>& element) { if (!nosqldb_.contains(key))nosqldb_[key] = element; return *this; }
	  NoSQLDB<T> add(const std::string& key, const DbElement<T>& element)const { if (!nosqldb_.contains(key))nosqldb_[key] = element; return *this; }

	  //method to delete key/value pair
	  NoSQLDB<T>& remove(const std::string& key) { if (nosqldb_.contains(key)) nosqldb_.rmele(key); return *this; }
	  NoSQLDB<T> remove(const std::string& key)const { if (nosqldb_.contains(key)) nosqldb_.rmele(key); return *this; }

	  //methond to append child to each value element
	  NoSQLDB<T> addchild(const std::string& parentkey, const std::string&childkey)const;
	  NoSQLDB<T>& addchild(const std::string& parentkey, const std::string&childkey);

	  //methond to remove child to each value element
	  NoSQLDB<T> rmchild(const std::string& parentkey, const std::string&childkey)const;
	  NoSQLDB<T>& rmchild(const std::string& parentkey, const std::string&childkey);

	  //method to modify specific value content
	  void modifydata(const std::string &key,const std::string& metadatatype, const std::string& after);
  private:
	  DbCore<T> nosqldb_;
  };

  //----<methond to append child to each value element>-----------------------------------
  template<typename T>
  NoSQLDB<T> NoSQLDB<T>::addchild(const std::string& parentkey, const std::string&childkey)const {
	  if (nosqldb_.contains(parentkey)) {
		  nosqldb_.getelement(parentkey).children().push_back(childkey);
	  }
	  return *this;
  }

  //----<methond to append child to each value element>-----------------------------------
  template<typename T>
  NoSQLDB<T>& NoSQLDB<T>::addchild(const std::string& parentkey, const std::string&childkey) {
	  if (nosqldb_.contains(parentkey)) {
		  nosqldb_.getelement(parentkey).children().push_back(childkey);
	  }
	  return *this;
  }

  //----<methond to remove child to each value element>-----------------------------------
  template<typename T>
  NoSQLDB<T> NoSQLDB<T>::rmchild(const std::string& parentkey, const std::string&childkey)const {
	  if (nosqldb_.contains(parentkey)) {
		  bool hasthechild = false;
		  int pos = -1;
		  DbElement<T> thekey = nosqldb_.getelement(parentkey);
		  for (auto item : thekey.children()) {
			  if (item == childkey) hasthechild = true;
		  }
		  if (hasthechild) {
			  for (auto item : thekey.children()) {
				  if (item != childkey)pos++;
			  }
			  nosqldb_.getelement(parentkey).children().erase(nosqldb_.getelement(parentkey).children().begin() + pos);
		  }
	  }
	  return *this;
  }

  //----<methond to remove child to each value element>-----------------------------------
  template<typename T>
  NoSQLDB<T>& NoSQLDB<T>::rmchild(const std::string& parentkey, const std::string&childkey) {
	  if (nosqldb_.contains(parentkey)) {
		  bool hasthechild = false;
		  int pos = -1;
		  DbElement<T> thekey = nosqldb_.getelement(parentkey);
		  for (auto item : thekey.children()) {
			  if (item == childkey) hasthechild = true;
		  }
		  if (hasthechild) {
			  for (auto item : thekey.children()) {
				  if (item != childkey)pos++;
			  }
			  nosqldb_.getelement(parentkey).children().erase(nosqldb_.getelement(parentkey).children().begin() + pos);
		  }
	  }
	  return *this;
  }
 
  //----<method to modify specific value content>-----------------------------------------
  template<typename T>
  void NoSQLDB<T>::modifydata(const std::string &key, const std::string& metadatatype, const std::string& after) {
	  if (nosqldb_.contains(key)) {
		  if (metadatatype == "name") nosqldb_.getelement(key).name(after);
		  else if (metadatatype == "description") nosqldb_.getelement(key).descrip(after);
		  else if (metadatatype == "datetime") nosqldb_.getelement(key).dateTime(after);
		  else if (metadatatype == "payload")nosqldb_.getelement(key).payLoad(after);
	  }
  }

  //----<method to show all nosql database content>----------------------------------------
  template<typename T>
  void shownosql(const NoSQLDB<T>& nosqldb, std::ostream& out = std::cout) {
	  out << "\n";
	  showHeader(out);
	  for (auto item : nosqldb.nosqldb()) {
		  showElem(item.second, out);
	  }
	  out << "\n";
  }

  /////////////////////////////////////////////////////////////////////
  // DbCore<T> methods

  //----< does db contain this key? >----------------------------------

  template<typename T>
  bool DbCore<T>::contains(const Key& key)
  {
    iterator iter = dbStore_.find(key);
    return iter != dbStore_.end();
  }
  //----< returns current key set for db >-----------------------------

  template<typename T>
  typename DbCore<T>::Keys DbCore<T>::keys()
  {
    DbCore<T>::Keys dbKeys;
    DbStore& dbs = dbStore();
    size_t size = dbs.size();
    dbKeys.reserve(size);
    for (auto item : dbs)
    {
      dbKeys.push_back(item.first);
    }
    return dbKeys;
  }
  //----< return number of db elements >-------------------------------

  template<typename T>
  size_t DbCore<T>::size()
  {
    return dbStore_.size();
  }
  //----< extracts value from db with key >----------------------------
  /*
  *  - indexes non-const db objects
  *  - In order to create a key-value pair in the database like this:
  *      db[newKey] = newDbElement
  *    we need to index with the new key and assign a default element. 
  *    That will be replaced by newDbElement when we return from operator[]
  *  - However, if we want to index without creating new elements, we need
  *    to throw an exception if the key does not exist in the database.
  *  - The behavior we get is determined by doThrow_.  If false we create
  *    a new element, if true, we throw. Creating new elements is the default
  *    behavior.
  */
  template<typename T>
  DbElement<T>& DbCore<T>::operator[](const Key& key)
  {
    if (!contains(key))
    {
      if (doThrow_)
        throw(std::exception("key does not exist in db"));
      else
        return (dbStore_[key] = DbElement<T>());
    }
    return dbStore_[key];
  }
  //----< extracts value from db with key >----------------------------
  /*
  *  - indexes const db objects
  */
  template<typename T>
  DbElement<T> DbCore<T>::operator[](const Key& key) const
  {
    if (!contains(key))
    {
      throw(std::exception("key does not exist in db"));
    }
    return dbStore_[key];
  }
  
  /////////////////////////////////////////////////////////////////////
  // display functions

  //----< display database key set >-----------------------------------

  template<typename T>
  void showKeys(DbCore<T>& db, std::ostream& out = std::cout)
  {
    out << "\n  ";
    for (auto key : db.keys())
    {
      out << key << " ";
    }
  }
  //----< show record header items >-----------------------------------

  inline void showHeader(std::ostream& out = std::cout)
  {
    out << "\n  ";
    out << std::setw(26) << std::left << "DateTime";
    out << std::setw(10) << std::left << "Name";
    out << std::setw(25) << std::left << "Description";
    out << std::setw(25) << std::left << "Payload";
    out << "\n  ";
    out << std::setw(26) << std::left << "------------------------";
    out << std::setw(10) << std::left << "--------";
    out << std::setw(25) << std::left << "-----------------------";
    out << std::setw(25) << std::left << "-----------------------";
  }
  //----< display DbElements >-----------------------------------------

  template<typename T>
  void showElem(const DbElement<T>& el, std::ostream& out = std::cout)
  {
    out << "\n  ";
    out << std::setw(26) << std::left << std::string(el.dateTime());
    out << std::setw(10) << std::left << el.name();
    out << std::setw(25) << std::left << el.descrip();
	out << std::setw(25) << std::left << el.payLoad();
   
    typename DbElement<T>::Children children = el.children();
    if (children.size() > 0)
    {
      out << "\n    child keys: ";
      for (auto key : children)
      {
        out << " " << key;
      }
    }
  }
}
