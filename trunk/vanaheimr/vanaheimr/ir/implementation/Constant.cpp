/*! \file   Constant.cpp
	\author Gregory Diamos <gregory.diamos@gatech.edu>
	\date   Thursday February 9, 2012
	\brief  The source file for the Constant family of classes.
*/

// Vanaheimr Includes
#include <vanaheimr/ir/interface/Constant.h>
#include <vanaheimr/ir/interface/Type.h>

#include <vanaheimr/compiler/interface/Compiler.h>

#include <vanaheimr/parser/interface/ConstantValueParser.h>

// Hydrazine Includes
#include <hydrazine/interface/debug.h>

// Standard Library Includes
#include <sstream>
#include <cstring>

namespace vanaheimr
{

namespace ir
{

Constant::Constant(const Type* type)
: _type(type)
{

}

Constant::~Constant()
{

}

const Type* Constant::type() const
{
	return _type;
}

Constant* Constant::parseConstantFromString(const std::string& value)
{
	std::stringstream stream(value);
	
	parser::ConstantValueParser parser;
	
	parser.parse(stream);

	return parser.parsedConstant()->clone();
}

const std::string toString(unsigned int bits)
{
	std::stringstream stream;
	
	stream << bits;
	
	return stream.str();
}

FloatingPointConstant::FloatingPointConstant(float f)
: Constant(compiler::Compiler::getSingleton()->getType("f32")), _float(f)
{

}

FloatingPointConstant::FloatingPointConstant(double d)
: Constant(compiler::Compiler::getSingleton()->getType("f64")), _double(d)
{

}

float FloatingPointConstant::asFloat() const
{
	return _float;
}

double FloatingPointConstant::asDouble() const
{
	return _double;
}

bool FloatingPointConstant::isNullValue() const
{
	return _double == 0.0;
}

Constant::DataVector FloatingPointConstant::data() const
{
	DataVector values(bytes());
	
	std::memcpy(values.data(), &_double, bytes());

	return values;
}

size_t FloatingPointConstant::bytes() const
{
	return type()->bytes();
}

Constant* FloatingPointConstant::clone() const
{
	return new FloatingPointConstant(*this);
}

IntegerConstant::IntegerConstant(uint64_t i, unsigned int bits)
: Constant(compiler::Compiler::getSingleton()->getType("i" + toString(bits)))
{

}

IntegerConstant::operator uint64_t() const
{
	return _value;
}

bool IntegerConstant::isNullValue() const
{
	return _value == 0;
}

Constant::DataVector IntegerConstant::data() const
{
	DataVector values(bytes());
	
	std::memcpy(values.data(), &_value, bytes());

	return values;
}

size_t IntegerConstant::bytes() const
{
	return type()->bytes();
}

Constant* IntegerConstant::clone() const
{
	return new IntegerConstant(*this);
}




ArrayConstant::ArrayConstant(const void* data, uint64_t size, const Type* t)
: Constant(t), _value((const uint8_t*)data, (const uint8_t*)data + size)
{

}

ArrayConstant::ArrayConstant(uint64_t size, const Type* t)
: Constant(t), _value(size)
{

}

Constant* ArrayConstant::getMember(unsigned int index)
{
	assertM(false, "Not implemented.");

	return nullptr;
}

const Constant* ArrayConstant::getMember(unsigned int index) const
{
	assertM(false, "Not implemented.");

	return nullptr;
}

uint64_t ArrayConstant::size() const
{
	return bytes() / type()->bytes();
}

bool ArrayConstant::isNullValue() const
{
	for(auto value : _value)
	{
		if(value != 0) return false;
	}
	
	return true;
}

Constant::DataVector ArrayConstant::data() const
{
	return _value;
}

size_t ArrayConstant::bytes() const
{
	return _value.size();
}

Constant* ArrayConstant::clone() const
{
	return new ArrayConstant(*this);
}

void* ArrayConstant::storage()
{
	return _value.data();
}

}

}

