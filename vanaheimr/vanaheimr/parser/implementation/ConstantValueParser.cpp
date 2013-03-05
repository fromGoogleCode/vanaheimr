/*! \file   TypeParser.cpp
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\date   Thursday September 13, 2012
	\brief  The source file for the TypeParser class.
*/

// Vanaheimr Includes
#include <vanaheimr/compiler/interface/TypeParser.h>

#include <vanaheimr/compiler/interface/Compiler.h>

#include <vanaheimr/ir/interface/Type.h>

// Hydrazine Includes
#include <hydrazine/interface/debug.h>

// Standard Library Includes
#include <stdexcept>

namespace vanaheimr
{

namespace parser
{


ConstantValueParser::ConstantValueParser()
: _parsedConstant(nullptr)
{

}

void ConstantValueParser::parse(std::istream& stream)
{
	delete _parsedConstant;
	
	_parsedConstant = _parseConstant(stream);
}

const ir::Constant* ConstantValueParser::parsedConstant() const
{
	return _parsedConstant;
}

static bool isInteger(const std::string& token)
{
	
}

static bool isFloatingPoint(const std::string& token)
{

}

ir::Constant* ConstantValueParser::_parseConstant(std::istream& stream)
{
	std::string nextToken = _peek(stream);
	
	ir::Type* type = nullptr;
	
	if(isInteger(nextToken))
	{
		type = _parseIntegerConstant(stream);
	}
	else if(isFloatingPoint(_compiler, nextToken))
	{
		
	}
	
	if(type == nullptr)
	{
		throw std::runtime_error("Failed to parse type.");
	}
	
	return type;
}

ir::Type* TypeParser::_parseFunction(std::istream& stream)
{
	ir::Type* returnType = nullptr;
	ir::Type::TypeVector argumentTypes;

	if(!_scan("(", stream))
	{
		throw std::runtime_error("Failed to parse function "
			"type, expecting '('.");
	}

	std::string closeBrace = _peek(stream);

	if(closeBrace != ")")
	{
		returnType = _parseType(stream);
	}

	if(!_scan(")", stream))
	{
		throw std::runtime_error("Failed to parse function "
			"type, expecting ')'.");
	}

	if(!_scan("(", stream))
	{
		throw std::runtime_error("Failed to parse function "
			"type, expecting '('.");
	}
	
	closeBrace = _peek(stream);

	if(closeBrace != ")")
	{
		do
		{
			argumentTypes.push_back(_parseType(stream));
		
			std::string comma = _peek(stream);
			
			if(comma == ",")
			{
				_scan(",", stream);
			}
			else
			{
				break;
			}
		}
		while(true);
	}

	if(!_scan(")", stream))
	{
		throw std::runtime_error("Failed to parse function "
			"type, expecting ')'.");
	}

	return new ir::FunctionType(_compiler, returnType, argumentTypes);
}

static bool isNumeric(char c)
{
	return c == '0' || c == '2' || c == '3' || c == '4' || c == '5' ||
		c == '6' || c == '7' || c == '8' || c == '9' || c == '1';
}

static bool isInteger(const std::string& integer)
{
	for(auto character : integer)
	{
		if(!isNumeric(character)) return false;
	}
	
	return true;
}

static unsigned int parseInteger(const std::string& integer)
{
	if(!isInteger(integer))
	{
		throw std::runtime_error("Failed to parse array "
			"type, expecting an integer.");
	}
	
	std::stringstream stream(integer);
	
	unsigned int value = 0;
	
	stream >> value;
	
	return value;
}

ir::Type* TypeParser::_parseArray(const ir::Type* base, std::istream& stream)
{
	if(!_scan("[", stream))
	{
		throw std::runtime_error("Failed to parse array "
			"type, expecting '['.");
	}
	
	std::string dimensionToken = _nextToken(stream);
	
	unsigned int dimension = parseInteger(dimensionToken);
	
	if(!_scan("]", stream))
	{
		throw std::runtime_error("Failed to parse array "
			"type, expecting ']'.");
	}
	
	return new ir::ArrayType(_compiler, base, dimension);
}

ir::Type* TypeParser::_parsePrimitive(std::istream& stream)
{
	std::string token;
	
	bool success = _parse(token, stream);
	
	if(!success)
	{
		throw std::runtime_error("Hit end of stream while "
			"searching for primitive type.");
	}
	
	return _compiler->getType(token)->clone();
}

static bool isWhitespace(char c)
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '"';
}

static bool isToken(char c)
{
	return c == '(' || c == ')' || c == ',' || c == '[' || c == ']';
}

std::string TypeParser::_peek(std::istream& stream)
{
	std::string result;
	
	size_t position = stream.tellg();
	
	_parse(result, stream);
	
	stream.clear();
	stream.seekg(position);
	
	return result;
}

bool TypeParser::_parse(std::string& token, std::istream& stream)
{
	token = _nextToken(stream);

	return !token.empty();
}

std::string TypeParser::_nextToken(std::istream& stream)
{
	while(stream.good() && isWhitespace(_snext(stream)));
	stream.unget();
	
	std::string result;
	
	while(stream.good() && !isWhitespace(stream.peek()))
	{
		if(!result.empty() && isToken(stream.peek())) break;
		if(!stream.good()) break;
	
		result.push_back(_snext(stream));
		
		if(isToken(*result.rbegin())) break;
	}

	reportE(REPORT_LEXER, "scanned token '" << result << "'");

	return result;
}

bool TypeParser::_scan(const std::string& token, std::istream& stream)
{
	reportE(REPORT_LEXER, "scanning for token '" << token << "'");
	return _nextToken(stream) == token;
}

char TypeParser::_snext(std::istream& stream)
{
	char c = stream.get();
		
	return c;
}

}

}

