// -*- mode: c++; c-basic-offset: 4; -*-

// Author: Hong Jiang <hong@hjiang.net>
// Contributors:
//   Amr Ali <amr@databracket.com>

#include "jsonxx.hh"

#include <cctype>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace jsonxx {

// Try to consume characters from the input stream and match the
// pattern string.
bool match(const char* pattern, std::istream& input) {
	input >> std::ws;
	const char* cur(pattern);
	char ch(0);
	while(input && !input.eof() && *cur != 0) {
		input.get(ch);
		if (ch != *cur) {
			input.putback(ch);
			return false;
		} else {
			cur++;
		}
	}
	return *cur == 0;
}

bool parse_string(std::istream& input, std::string* value) {
	if (!match("\"", input))  {
		return false;
	}
	char ch;
	while(!input.eof() && input.good()) {
		input.get(ch);
		if (ch == '"') {
			break;
		}
		if (ch == '\\') {
			input.get(ch);
			switch(ch) {
				case '"':
				case '\\':
				case '/':
					value->push_back(ch);
					break;
				case 'b':
					value->push_back('\b');
					break;
				case 'f':
					value->push_back('\f');
					break;
				case 'n':
					value->push_back('\n');
					break;
				case 'r':
					value->push_back('\r');
					break;
				case 't':
					value->push_back('\t');
					break;
				default:
					value->push_back('\\');
					value->push_back(ch);
					break;
			}
		} else {
			value->push_back(ch);
		}
	}
	if (input && ch == '"') {
		return true;
	} else {
		return false;
	}
}

bool parse_bool(std::istream& input, bool* value) {
	if (match("true", input))  {
		*value = true;
		return true;
	}
	if (match("false", input)) {
		*value = false;
		return true;
	}
	return false;
}

bool parse_null(std::istream& input) {
	if (match("null", input))  {
		return true;
	}
	return false;
}

bool parse_number(std::istream& input, double* value) {
	input >> std::ws;
	input >> *value;
	if (input.fail()) {
		input.clear();
		return false;
	}
	return true;
}

Object::Object() : value_map_() {}

Object::~Object() {
	std::map<std::string, Value*>::iterator i;
	for (i = value_map_.begin(); i != value_map_.end(); ++i) {
		delete i->second;
	}
}

bool Object::parse(std::istream& input, Object& object) {
	object.value_map_.clear();

	if (!match("{", input)) {
		return false;
	}

	do {
		std::string key;
		if (!parse_string(input, &key)) {
			return false;
		}
		if (!match(":", input)) {
			return false;
		}
		Value* v = new Value();
		if (!Value::parse(input, *v)) {
			delete v;
			break;
		}
		object.value_map_[key] = v;
	} while (match(",", input));

	if (!match("}", input)) {
		return false;
	}
	return true;
}

Value::Value() : type_(INVALID_) {}

void Value::reset() {
	if (type_ == STRING_) {
		delete string_value_;
	}
	if (type_ == OBJECT_) {
		delete object_value_;
	}
	if (type_ == ARRAY_) {
		delete array_value_;
	}
}

bool Value::parse(std::istream& input, Value& value) {
	value.reset();

	std::string string_value;
	if (parse_string(input, &string_value)) {
		value.string_value_ = new std::string();
		value.string_value_->swap(string_value);
		value.type_ = STRING_;
		return true;
	}
	if (parse_number(input, &value.number_value_)) {
		value.type_ = NUMBER_;
		return true;
	}

	if (parse_bool(input, &value.bool_value_)) {
		value.type_ = BOOL_;
		return true;
	}
	if (parse_null(input)) {
		value.type_ = NULL_;
		return true;
	}
	if (input.peek() == '[') {
		value.array_value_ = new Array();
		if (Array::parse(input, *value.array_value_)) {
			value.type_ = ARRAY_;
			return true;
		}
		delete value.array_value_;
	}
	value.object_value_ = new Object();
	if (Object::parse(input, *value.object_value_)) {
		value.type_ = OBJECT_;
		return true;
	}
	delete value.object_value_;
	return false;
}

template<>
Value * Value::from(const bool& value) {
  Value *ret = new Value;
  ret->type_ = BOOL_;
  ret->bool_value_ = value;
  return ret;
}

template<>
Value * Value::from(const double& value) {
  Value *ret = new Value;
  ret->type_ = NUMBER_;
  ret->number_value_ = value;
  return ret;
}

template<>
Value * Value::from(const std::string& value) {
  Value *ret = new Value;
  ret->type_ = STRING_;
  ret->string_value_ = new std::string(value);
  return ret;
}

template<>
Value * Value::from(const Value::Null&) {
  Value *ret = new Value;
  ret->type_ = NULL_;
  return ret;
}

template<>
Value * Value::from(const Array& value) {
  Value *ret = new Value;
  ret->type_ = ARRAY_;
  ret->array_value_ = new Array;

  const std::vector<Value*>& ref = value.values();

  for (std::vector<Value*>::const_iterator it = ref.begin();
		  it != ref.end(); ++it) {

	  if ((*it)->is<Value::Null>())
		  ret->array_value_->add(Value::Null());

	  if ((*it)->is<bool>())
		  ret->array_value_->add((*it)->get<bool>());

	  if ((*it)->is<std::string>())
		  ret->array_value_->add((*it)->get<std::string>());

	  if ((*it)->is<double>())
		  ret->array_value_->add((*it)->get<double>());

	  if ((*it)->is<Array>())
		  ret->array_value_->add((*it)->get<Array>());

	  if ((*it)->is<Object>())
		  ret->array_value_->add((*it)->get<Object>());
  }

  return ret;
}

template<>
Value * Value::from(const Object& value) {
  Value *ret = new Value;
  ret->type_ = OBJECT_;
  ret->object_value_ = new Object;

  const std::map<std::string, Value*>& ref = value.kv_map();

  for (std::map<std::string, Value*>::const_iterator it = ref.begin();
		  it != ref.end(); ++it) {

	  if ((*it).second->is<Value::Null>())
		  ret->object_value_->set((*it).first, Value::Null());

	  if ((*it).second->is<bool>())
		  ret->object_value_->set((*it).first, (*it).second->get<bool>());

	  if ((*it).second->is<std::string>())
		  ret->object_value_->set((*it).first, (*it).second->get<std::string>());

	  if ((*it).second->is<double>())
		  ret->object_value_->set((*it).first, (*it).second->get<double>());

	  if ((*it).second->is<Array>())
		  ret->object_value_->set((*it).first, (*it).second->get<Array>());

	  if ((*it).second->is<Object>())
		  ret->object_value_->set((*it).first, (*it).second->get<Object>());
  }

  return ret;
}

Array::Array() : values_() {}

Array::~Array() {
	for (unsigned int i = 0; i < values_.size(); ++i) {
		delete values_[i];
	}
}

bool Array::parse(std::istream& input, Array& array) {
	array.values_.clear();

	if (!match("[", input)) {
		return false;
	}

	do {
		Value* v = new Value();
		if (!Value::parse(input, *v)) {
			delete v;
			break;
		}
		array.values_.push_back(v);
	} while (match(",", input));

	if (!match("]", input)) {
		return false;
	}
	return true;
}

static std::ostream& stream_string(std::ostream& stream,
								   const std::string& string) {
	stream << '"';
	for (std::string::const_iterator i = string.begin(),
				 e = string.end(); i != e; ++i) {
		switch (*i) {
			case '"':
				stream << "\\\"";
				break;
			case '\\':
				stream << "\\\\";
				break;
			case '/':
				stream << "\\/";
				break;
			case '\b':
				stream << "\\b";
				break;
			case '\f':
				stream << "\\f";
				break;
			case '\n':
				stream << "\\n";
				break;
			case '\r':
				stream << "\\r";
				break;
			case '\t':
				stream << "\\t";
				break;
			default:
				if (*i < 32) {
					stream << "\\u" << std::hex << std::setw(6) <<
							std::setfill('0') << static_cast<int>(*i) << std::dec <<
							std::setw(0);
				} else {
					stream << *i;
				}
		}
	}
	stream << '"';
	return stream;
}

}  // namespace jsonxx

std::ostream& operator<<(std::ostream& stream, const jsonxx::Value& v) {
	if (v.is<double>()) {
		return stream << v.get<double>();
	} else if (v.is<std::string>()) {
		return jsonxx::stream_string(stream, v.get<std::string>());
	} else if (v.is<bool>()) {
		if (v.get<bool>()) {
			return stream << "true";
		} else {
			return stream << "false";
		}
	} else if (v.is<jsonxx::Value::Null>()) {
		return stream << "null";
	} else if (v.is<jsonxx::Object>()) {
		return stream << v.get<jsonxx::Object>();
	} else if (v.is<jsonxx::Array>()){
		return stream << v.get<jsonxx::Array>();
	}
	// Shouldn't reach here.
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const jsonxx::Array& v) {
	stream << "[";
	const std::vector<jsonxx::Value*>& values(v.values());
	for (unsigned int i = 0; i < values.size();) {
		stream << *(values[i]);
		if (++i < values.size())
			stream << ", ";
	}
	return stream << "]";
}

std::ostream& operator<<(std::ostream& stream, const jsonxx::Object& v) {
	stream << "{";
	const std::map<std::string, jsonxx::Value*>& kv(v.kv_map());
	for (std::map<std::string, jsonxx::Value*>::const_iterator i = kv.begin();
		 i != kv.end(); /**/) {
		jsonxx::stream_string(stream, i->first);
		stream << ": " << *(i->second);
		++i;
		if ( i != kv.end()) {
			stream << ", ";
		}
	}
	return stream << "}";
}
