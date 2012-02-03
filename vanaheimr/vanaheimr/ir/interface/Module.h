/*! \file   Module.h
	\date   Friday February 3, 2012
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\brief  The header file for the Module class.
*/

#pragma once

/*! \brief The wrapper namespace for Vanaheimr */
namespace vanaheimr
{

/*! \brief A namespace for the internal representation */
namespace ir
{

/*! \brief Represents a single compilation unit. */
class Module
{
public:
	typedef std::list<Function> FunctionList;
	typedef std::list<Global>   GlobalList;

	typedef FunctionList::iterator       iterator;
	typedef FunctionList::const_iterator const_iterator;
	typedef GlobalList::iterator         global_iterator;
	typedef GlobalList::const_iterator   const_global_iterator;

public:
	/*! \brief Create a new module with the specified name */
	Module(const std::string& name);
	~Module();
	
public:
	/*! \brief Get a named function in the module, return 0 if not found */
	Function* getFunction(const std::string& name);

	/*! \brief Get a named function in the module, return 0 if not found */
	const Function* getFunction(const std::string& name) const;
	
	/*! \brief Insert a function into the module, it takes ownership */
	void insertFunction(Function* f);

	/*! \brief Remove a function from the module, it is not deleted */
	void removeFunction(Function* f);

public:
	/*! \brief Get a named global in the module, return 0 if not found */
	Global* getGlobal(const std::string& name);

	/*! \brief Get a named global in the module, return 0 if not found */
	const Global* getGlobal(const std::string& name) const;
	
	/*! \brief Insert a global into the module, it takes ownership */
	void insertGlobal(Global* g);

	/*! \brief Remove a global from the module, it is not deleted */
	void removeGlobal(Global* g);

public:
	/*! \brief Write the module to a binary */
	void writeBinary(std::ostream&);

	/*! \brief Write the module as IR to an assembly file */
	void writeAssembly(std::ostream&);

public:
	//! Function Iteration
	iterator       begin();
	const_iterator begin() const;

	iterator       end();
	const_iterator end() const;

public:
	//! Function info
	size_t size() const;
	bool empty() const;

public:
	//! Function Iteration
	global_iterator       global_begin();
	const_global_iterator global_begin() const;

	global_iterator       global_end();
	const_global_iterator global_end() const;

public:
	//! Global info
	size_t global_size() const;
	bool global_empty() const;
	
public:
	std::string name;
	
private:
	FunctionList _functions;
	GlobalList   _globals;

};

}

}

