/* ====================================================================================
* Projekt: PYEKVS(Pye - Key - Value - Storage)
* Description : simple key value serialization format
* Compiler : C++14 ISO
* Author : Dr. Sylvio Schneider
* Version 2.0
* License : MIT
* ====================================================================================
* Copyright(c) 2021 Dr. Sylvio Schneider

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files(the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :

* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ====================================================================================
* pyeKVS format specification :
* https://kxtec.de/projects/pyekvs/specification
* pyeKVS_Spec.md
* Version 1.0
* ====================================================================================
* Pascal API implementation :
* https://kxtec.de/projects/pyekvs/delphi_api
* Readme.md
* Version : 1 from 01.03.2021 for pyeKVS V 1.0
* - initial release
* Version : 2 from 14.03.2021 for pyeKVS V 1.0
* - update pyeList to use external interfaces
* - update pyeDocument
* - New constructor to define custom container classes for
* pyeKVSList, pyeKVSArrayand pyeKVSArrayMap
* User can overwrite these classes to e.g.define own interfaces.
* - New constructor to define custom stream classes.Useful to write values
* direct in userdef stream class
* ====================================================================================
*/

#include <string>
#include <sstream>
#include <stdint.h>
#include <algorithm>
#include <fstream>

#pragma once

#define DATASIZE 1024 

/// <summary> Typedef for 128-bit integer </summary>
typedef std::vector<unsigned char> int128;
/// <summary> Typedef for 128-bit unsigned integer </summary>
typedef std::vector<unsigned char> uInt128;
/// <summary> Typedef for 128-bit float </summary>
typedef std::vector<unsigned char> float128;


/// <summary>
/// Several string components for printing formatted pyeKVS on the screen.
/// </summary>
const char Str_PYEKVSSpace = ' ';
const char Str_PYEKVSStringQuote = '\"';
const char Str_PYEKVSComma = ',';
const char Str_PYEKVSTypeSeperatorO = '(';
const char Str_PYEKVSTypeSeperatorC = ')';
const char Str_PYEKVSValueSeperator = ':';
const char Str_PYEKVSArraySeperatorO = '[';
const char Str_PYEKVSArraySeperatorC = ']';
const char Str_PYEKVSArrayTypeOf[] = "of";
const char Str_PYEKVSArrayCount[] = "Count";
const char Str_PYEKVSListSeperatorO = '{';
const char Str_PYEKVSListSeperatorC = '}';
const char Str_PYEKVSValueBinary[] = "Len:%d Data:$%x";
const char Str_PYEKVSUnknown[] = "Unknown value type";
const char Str_PYEKVSZero = '0';
const char Str_PYEKVSBool = '1';

/// <summary>
/// Enum of pyeKVS value types
/// see more: https://www.kxtec.de/project/pyekvs/pyekvs-specification/
/// </summary>
enum pyeValueType : unsigned __int8 {
	pyeUnknown,
	pyeList,         // dynamic length; special header 4Bytes Size + 4Bytes Count
	pyeZero,         //  0 Bytes; Bool=false; Int=0; Float=0; UFT8=''; Mem=nil
	pyeBool,         //  0 Bytes; Bool=true
	pyeInt8,         //  1 Bytes; little endian;	-128 - 127
	pyeUInt8,        //  1 Bytes; little endian;    0 - 255
	pyeInt16,        //  2 Bytes; little endian;	-32.768 - 32.767
	pyeUInt16,       //  2 Bytes; little endian;	0 - 65.535
	pyeInt32,        //  4 Bytes; little endian;	-2.147.483.648 - 2.147.483.647
	pyeUInt32,       //  4 Bytes; little endian;	0 - 4.294.967.295
	pyeInt64,        //  8 Bytes; little endian;	-9.223.372.036.854.775.808 -  9.223.372.036.854.775.807
	pyeUInt64,       //  8 Bytes; little endian;	0 - 18.446.744.073.709.551.615
	pyeInt128,       // 16 Bytes; little endian;	-1,70141E38 - 1,70141E38
	pyeUInt128,      // 16 Bytes; little endian;	0 - 3,40282E38
	pyeFloat32,      //  8 Bytes; little endian;	single
	pyeFloat64,      // 16 Bytes; little endian;	double
	pyeFloat128,     // 32 Bytes; little endian;	Reserve
	pyeStringUTF8S,  // dynamic length; special header S-mall 1Byte char count + string as UTF8 chars; max 255 chars
	pyeStringUTF8L,  // dynamic length; special header L-arge 4Byte char count + string as UTF8 chars; max 2147483647 chars
	pyeMemory,       // dynamic length; special header 4Byte mem size + values as byte stream
	pyeArray,        // dynamic length; special header 1Byte ValueType + 4Bytes Size + 4Bytes Count + values
	pyeArrayMap      // dynamic length; special header 2Byte map length + map +  4Bytes Size + 4Bytes Count + values
};

/// <summary>
/// Array of strings of pyeKVS value type names for messaging in the log
/// </summary>
const std::string pyeKVSValueTypeName[22] = {
	"Unknown",
	"List",
	"Zero",
	"Bool",
	"Int8",
	"UInt8",
	"Int16",
	"UInt16",
	"Int32",
	"UInt32",
	"Int64",
	"UInt64",
	"Int128",
	"UInt128",
	"Float32",
	"Float64",
	"Float128",
	"StringUTF8S",
	"StringUTF8L",
	"Memory",
	"Array",
	"ArrayMap"
};

/// <summary>
/// Reads a byte sequence from a vector.
/// </summary>
/// <typeparam name="T">Data type to read from vector</typeparam>
/// <param name="t">Reference, where to write the byte sequence to</param>
/// <param name="v">Vector of bytes, where to read the byte sequence from</param>
/// <param name="offset">Index, where to start to read from vector</param>
template <class T>
void ReadFromVector(T& t, std::vector<unsigned char>& v, std::size_t offset) {
	memcpy(&t, &v[offset], sizeof(T));
};

/// <summary>
/// Writes a byte sequence to a vector.
/// </summary>
/// <typeparam name="T">Data yype to write to vector</typeparam>
/// <param name="t">Reference, that holds the data to write to the vector</param>
/// <param name="v">Vector of bytes, where to write the byte sequence to</param>
/// <param name="offset">Index, where to start to write to vector</param>
template <class T>
void WriteToVector(std::vector<unsigned char>& v, T& t, std::size_t offset) {
	unsigned __int64 lastpos = offset + sizeof(T);
	if (v.size() < lastpos) {
		v.resize(lastpos);
	}
	
	memcpy(&v[offset], &t, sizeof(T));
};

/// <summary>
/// Get the size of a advanced pyeKVS type. 
/// </summary>
/// <param name="buffer"></param>
/// <param name="offset"></param>
/// <param name="valueType"></param>
/// <returns></returns>
unsigned __int32 getSizeOfAdvancedValueType(std::vector<unsigned __int8>& buffer, unsigned __int64 offset, unsigned __int8 valueType);

/// <summary>
/// Get the size of a fundamental pyeKVS type. 
/// </summary>
/// <param name="valueType"></param>
/// <returns></returns>
unsigned __int8 getSizeOfFundamentalValueType(pyeValueType valueType);

/// <summary>
/// Holds the pointer to the byte buffer and the operations for the 
/// fundamental pyeKVS types.
/// </summary>
/// <typeparam name="T">Data type of the key. Can be a string in case of pyeList 
/// or a unsigned __int32 in case of pyeArray or pyeMap</typeparam>
template<typename T>
class PyeBase {
	/// <summary> Pointer to the byte buffer </summary>
	std::vector<unsigned char> *_buffer;

public:
	/// <summary>
	/// Encode pyeKVS objects to byte stream.
	/// </summary>
	virtual void encode() {}

	/// <summary>
	/// Decode byte stream to pyeKVS objects.
	/// </summary>
	virtual void decode() {}

	/// <summary>
	/// Set the pointer to the byte buffer.
	/// </summary>
	/// <param name="buffer"></param>
	void setBuffer(std::vector<unsigned char>* buffer) {
		_buffer = buffer;
	};

	/// <summary>
	/// Get the pointer to the byte buffer. 
	/// </summary>
	/// <returns></returns>
	std::vector<unsigned char>* getBuffer() {
		return _buffer;
	};
	
	/// <summary>
	/// Set the offset to the first byte of a pyeKVS object in the pyeKVS data stream.
	/// </summary>
	/// <param name="offset">index position as offset</param>
	virtual void setOffsetObject(unsigned __int64 offset) = 0;

	/// <summary>
	/// Get the offset to the first byte of a pyeKVS object in the pyeKVS data stream. 
	/// </summary>
	/// <returns>index position as offset</returns>
	virtual unsigned __int64 getOffsetObject() = 0;

	/// <summary>
	/// Get the value of the size (amount of bytes) of a pyeKVS key.
	/// </summary>
	/// <returns>size of the key</returns>
	unsigned __int8 getKeySize() {
		unsigned __int8 keySize;
		ReadFromVector(keySize, *getBuffer(), getOffsetObject());
		return keySize;
	};

	/// <summary>
	/// Get the key of a pyeKVS key-value-pair.
	/// </summary>
	/// <returns>key</returns>
	std::string getKey() {
		unsigned __int8 keySize = getKeySize();
		unsigned __int64 offset = getOffsetObject() + 1 /*information key size (1 byte)*/;
		std::string keyString(&_buffer[offset], &_buffer[offset] + keySize);
		return keyString;
	};

	/// <summary>
	/// Get the offset (index position) of a pyeKVS value of a pyeKVS key-value-pair.
	/// </summary>
	/// <returns>offset</returns>
	unsigned __int64 getOffsetValue() {
		unsigned __int64 offset = getOffsetObject() + 1 /*information size of key*/;
		offset += getKeySize();
		return offset;
	}
	
	/// <summary>
	/// Reads the pyeKVS value type of the key-value-pair in the byte stream.
	/// </summary>
	/// <returns>pyeValueType</returns>
	pyeValueType getValueType() {
		pyeValueType _pyeValueType;
		ReadFromVector(_pyeValueType, *getBuffer(), getOffsetValue());
		return _pyeValueType;
	};

	/// <summary>
	/// Get size (amount of bytes) of the value (only array, list and map).
	/// </summary>
	/// <returns>Amount of bytes</returns>
	virtual unsigned __int32 getSize() = 0;
	
	/// <summary>
	/// Get count of pyeKVS objects of the pyKVS value (only array, list and map).
	/// </summary>
	/// <returns>Count of pyeKVS objects</returns>
	virtual unsigned __int32 getCount() = 0;

	/// <summary>
	/// Put a 'Zero value' to the pyeKVS data. 
	/// In this special case, only the key and the pyeKVS data type pyeZero will be added to the pykeKVS byte stream without any value.
	/// </summary>
	/// <param name="key">Key name</param>
	/// <returns>this</returns>
	PyeBase& putZero(T key) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeZero;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary> 
	/// Put a 'Zero value' to the pyeKVS data. 
	/// In this special case, only the key and the pyeKVS data type pyeBool will be added to the pykeKVS byte stream without any value.
	/// </summary>
	/// <param name="key">Key name</param>
	/// <returns>pyeKVS class itself</returns>
	PyeBase& putBool(bool value, T key = NULL) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeBool;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}
	
	/// <summary> 
	/// Puts an Int8 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">int8 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putInt8(__int8 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeInt8;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an Int16 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">int16 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putInt16(__int16 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeInt16;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an Int32 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">int32 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putInt32(__int32 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeInt32;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an Int64 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">int64 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putInt64(__int64 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeInt64;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an Int128 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">int128 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putInt128(int128 value, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeInt128;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		for (int i = 0; i < value.length(); i++) {
			WriteToVector(*getBuffer(), value[i], (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an unsigned Int8 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">unsigned int8 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putUInt8(unsigned __int8 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeUInt8;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an unsigned Int16 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">unsigned int16 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putUInt16(unsigned __int16 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeUInt16;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an unsigned Int32 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">unsigned int32 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putUInt32(unsigned __int32 value = 0, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeUInt32;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an unsigned Int64 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">unsigned int64 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putUInt64(unsigned __int64 value, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeUInt64;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts an unsigned Int128 value to the pyeKVS data.
	/// </summary>
	/// <param name="value">unsigned int128 value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putUInt128(uInt128 value, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeUInt128;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		for (int i = 0; i < value.length(); i++) {
			WriteToVector(*getBuffer(), value[i], (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts a float value to the pyeKVS data.
	/// </summary>
	/// <param name="value">float value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putFloat(float value, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeFloat32;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts a 128bit float value to the pyeKVS data.
	/// </summary>
	/// <param name="value">128bit float value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putFloat128(float128 value, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeFloat128;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		for (int i = 0; i < value.length(); i++) {
			WriteToVector(*getBuffer(), value[i], (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts a double value to the pyeKVS data.
	/// </summary>
	/// <param name="value">double value</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putDouble( double value, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeFloat64;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}
		WriteToVector(*getBuffer(), value, (*getBuffer()).size());

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts a short string to the pyeKVS data.
	/// </summary>
	/// <param name="shortString">short string</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putStringS(std::string shortString, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeStringUTF8S;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		unsigned __int8 stringLength = shortString.length();
		WriteToVector(*getBuffer(), stringLength, (*getBuffer()).size());

		for (int i = 0; i < shortString.length(); i++) {
			WriteToVector(*getBuffer(), shortString[i], (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts a long string to the pyeKVS data.
	/// </summary>
	/// <param name="longString">long string</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putStringL(std::string longString, std::string key = std::string()) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeStringUTF8L;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		unsigned __int32 stringLength = longString.length();
		WriteToVector(*getBuffer(), stringLength, (*getBuffer()).size());

		for (int i = 0; i < longString.length(); i++) {
			WriteToVector(*getBuffer(), longString[i], (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}

	/// <summary>
	/// Puts a byte stream to the pyeKVS data.
	/// </summary>
	/// <param name="memory">byte stream</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	PyeBase& putMemory(std::vector<unsigned char> memory = "", std::string key = NULL) {
		writeKeyToBuffer(*getBuffer(), key);
		if (!key.empty()) {
			unsigned __int8 dataType = pyeValueType::pyeMemory;
			WriteToVector(*getBuffer(), dataType, (*getBuffer()).size());
		}

		unsigned __int32 memory = longString.length();
		WriteToVector(*getBuffer(), memory, (*getBuffer()).size());

		for (int i = 0; i < memory.length(); i++) {
			WriteToVector(*getBuffer(), memory[i], (*getBuffer()).size());
		}

		updateObjectHeader();
		updateHeaderSize();

		return *this;
	}
	
	/// <summary>
	/// Gets a zero value. In this special case, if the pyeKVS data type is pyeZero, the method returns 
	/// </summary>
	/// <param name="key"></param>
	/// <param name="mapRowItem"></param>
	/// <returns></returns>
	bool getZero(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int8 result;
		unsigned __int64 offset = getOffsetItem(key);
		offset--;
		ReadFromVector(result, *getBuffer(), offset);

		//result=false means Key is vtZero
		//result=true means Key <> vtZero
		return (result != pyeValueType::pyeZero);
	}

	/// <summary>
	/// Gets a zero value. In this special case, if the key exists 
	/// </summary>
	/// <param name="value">byte stream</param>
	/// <param name="key">key name</param>
	/// <returns>this</returns>
	bool getBool(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int8 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		offset--;
		ReadFromVector(result, *getBuffer(), offset);

		switch (result) {
		case pyeValueType::pyeZero: return false;
		case pyeValueType::pyeBool: return true;
		default: return NULL;
		}
	}

	/// <summary>
	/// Gets a int8 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>int8 value</returns>
	__int8 getInt8(T key, unsigned __int16 mapRowItem = 0) {
		__int8 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a int16 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>int16 value</returns>
	__int16 getInt16(T key, unsigned __int16 mapRowItem = 0) {
		__int16 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a int32 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>int32 value</returns>
	__int32 getInt32(T key, unsigned __int16 mapRowItem = 0) {
		__int32 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a int64 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>int64 value</returns>
	__int64 getInt64(T key, unsigned __int16 mapRowItem = 0) {
		__int64 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a int128 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>int128 value</returns>
	int128 getInt128(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		int128 result((*getBuffer()).begin() + offset, (*getBuffer()).begin() + offset + 16);
		return result;
	}

	/// <summary>
	/// Gets a unsigned int8 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>unsigned int8 value</returns>
	unsigned __int8 getUInt8(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int8 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a unsigned int16 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>unsigned int16 value</returns>
	unsigned __int16 getUInt16(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int16 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a unsigned int32 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>unsigned int32 value</returns>
	unsigned __int32 getUInt32(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int32 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a unsigned int64 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>unsigned int64 value</returns>
	unsigned __int64 getUInt64(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a unsigned int128 value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>unsigned int128 value</returns>
	uInt128 getUInt128(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);

		uInt128 result((*getBuffer()).begin() + offset, (*getBuffer()).begin() + offset + 16);
//		result.shrink_to_fit();

		return result;
	}

	/// <summary>
	/// Gets a float value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>float value</returns>
	float getFloat(T key, unsigned __int16 mapRowItem = 0) {
		float result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a float 128-bit value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>float 128-bit value</returns>
	float128 getFloat128(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);

		float128 result((*getBuffer()).begin() + offset, (*getBuffer()).begin() + offset + 16);
//		result.shrink_to_fit();

		return result;
	}

	/// <summary>
	/// Gets a double value from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>double value</returns>
	double getDouble(T key, unsigned __int16 mapRowItem = 0) {
		double result;
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);
		ReadFromVector(result, *getBuffer(), offset);
		return result;
	}

	/// <summary>
	/// Gets a short string from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>short string</returns>
	std::string getStringS(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);

		unsigned __int8 stringSize;
		ReadFromVector(stringSize, *getBuffer(), offset);

		offset += sizeof(stringSize);

		std::string dataString((*getBuffer()).begin() + offset, (*getBuffer()).begin() + offset + stringSize);

		return dataString;
	}

	/// <summary>
	/// Gets a long string from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>long string</returns>
	std::string getStringL(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);

		unsigned __int32 stringSize;
		ReadFromVector(stringSize, *getBuffer(), offset);

		offset += sizeof(stringSize);

		std::string dataString((*getBuffer()).begin() + offset, (*getBuffer()).begin() + offset + stringSize);

		return dataString;
	}

	/// <summary>
	/// Gets a byte stream from the pyeKVS byte stream. 
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns>byte stream</returns>
	std::vector<unsigned char> getMemory(T key, unsigned __int16 mapRowItem = 0) {
		unsigned __int64 offset = getOffsetItem(key, mapRowItem);

		unsigned __int32 memSize;
		ReadFromVector(memSize, *getBuffer(), offset);

		offset += sizeof(memSize);

		std::vector<unsigned char> result((*getBuffer()).begin() + offset, (*getBuffer()).begin() + offset + memSize);
//		result.shrink_to_fit();

		return result;
	}

	/// <summary>
	/// Updates the size value in the header of the pyeKVS byte stream.
	/// </summary>
	void updateHeaderSize() {
		unsigned __int64 nweSize = (*getBuffer()).size() - 16;
		WriteToVector(*getBuffer(), nweSize, 8);
	}

private:
	/// <summary>
	/// Gets offset of a item in a pyeList, pyeArray or pyeArrayMap.
	/// </summary>
	/// <param name="key">key name</param>
	/// <param name="mapRowItem">row count, only needed in case of pyeArrayMap</param>
	/// <returns></returns>
	virtual unsigned __int64 getOffsetItem(T key, unsigned __int16 mapRowItem) = 0;

	/// <summary>
	/// Updates the header of the pyeKVS object.
	/// </summary>
	virtual void updateObjectHeader() = 0;

	/// <summary>
	/// Writes the key to the byte stream.
	/// </summary>
	/// <param name="buffer">byte stream</param>
	/// <param name="key">key name</param>
	/// <returns>size of the byte stream</returns>
	virtual unsigned __int64 writeKeyToBuffer(std::vector<unsigned char>& buffer, std::string key) {
		return buffer.size();
	}
};

// forward declaration
class PyeList;

/*
* pyeArray
* Name		type			size in byte	usage
* pyeArray	pyeValueType	1				Value=20
* ArrayType	pyeValueType	1				value type of the items
* Size		UInt32			4				Size of the array in bytes; start after header
* Count		UInt32			4				count of items in the array
* Item0		pyeValue
* Item1		pyeValue
* Item…		pyeValue

* The pyeArray can contain the following pyeValueType:

* pyeInt8
* pyeUInt8
* pyeInt16
* pyeUInt16
* pyeInt32
* pyeUInt32
* pyeInt64
* pyeUInt64
* pyeInt128
* pyeUInt128
* pyeFloat32
* pyeFloat64
* pyeFloat128
* pyeStringUTF8S (dynamic length!)
* pyeStringUTF8L (dynamic length!)
* pyeMemory (dynamic length!)

* StreamPos[array value data start] + Index * SizeOf(array data type)
*/

/// <summary>
/// The pyeArray is a container for further values given by the pyeValue definition. 
/// The individual items do not have an individual key.
/// The array header starts with 1 byte for the value type information of the array items.
/// Then follow 4 bytes size in bytes and 4 bytes count of items.
/// This information is strictly speaking duplicated.
/// But it is useful in decoding the stream to perform an internal validation.
/// </summary>
class PyeArray : public PyeBase<unsigned __int32> {
	PyeList *_pLastList = nullptr;
	unsigned __int64 _offsetObject = 0;
	unsigned __int32 _cntItems = 0;

public:
	/// <summary>
	/// Constructor of pyeArray
	/// </summary>
	/// <param name="pLastList">parent pyeList of the pyeArray</param>
	/// <param name="arrayType">pyeKVS type of the items</param>
	/// <param name="offsetObjectStart">offset of the pyeArray object in the byte stream</param>
	PyeArray(PyeList* pLastList, pyeValueType arrayType, unsigned __int64 offsetObjectStart);

	/// <summary>
	/// Constructor of pyeArray
	/// </summary>
	/// <param name="buffer">pointer to the byte stream</param>
	/// <param name="offsetObjectStart">offset of the pyeArray object in the byte stream</param>
	PyeArray(std::vector<unsigned char>* buffer, unsigned __int64 offsetObjectStart) {
		setOffsetObject(offsetObjectStart);
		setBuffer(buffer);
	}

	/// <summary>
	/// Gets pyeKVS value type of the items in the array.
	/// </summary>
	/// <returns>pye value type</returns>
	pyeValueType getArrayDataType() {
		unsigned __int8 arrayValueType;
		ReadFromVector(arrayValueType, *getBuffer(), getOffsetValue() + 1 /*pyeValueType(1 byte)*/);

		return (pyeValueType) arrayValueType;
	}

	/// <summary>
	/// Gets the size (amount of bytes) of the array.
	/// </summary>
	/// <returns>size of the array</returns>
	virtual unsigned __int32 getSize() {
		unsigned __int32 arraySize;
		ReadFromVector(arraySize, *getBuffer(), getOffsetValue() + 2 /*pyeValueType(1 byte) + information size of arrayValueType(1 byte)*/);
		return arraySize;
	}

	/// <summary>
	/// Gets the count of items of the array.
	/// </summary>
	/// <returns>count of items</returns>
	virtual unsigned __int32 getCount() {
		unsigned __int32 arrayCount;
		ReadFromVector(arrayCount, *getBuffer(), getOffsetValue() + 6 /*pyeValueType(1 byte) + information size of arrayValueType(1 byte) + arraySize(4 byte)*/);
		return arrayCount;
	}

	/// <summary>
	/// Sets the offset of the object in the byte stream.
	/// </summary>
	/// <param name="offset"></param>
	virtual void setOffsetObject(unsigned __int64 offset) {
		_offsetObject = offset;
	};
	
	/// <summary>
	/// Gets the offset of the object in the byte stream.
	/// </summary>
	/// <returns>offset</returns>
	virtual unsigned __int64 getOffsetObject() {
		return _offsetObject;
	}

	/// <summary>
	/// Generates a JSON-formatted string of the pyeArray.
	/// </summary>
	/// <returns>string</returns>
	std::string toStringJSON() {
		unsigned __int8 itemValueType = getArrayDataType();
		unsigned __int32 count = getCount();

		std::string data = "[";

		for (unsigned __int32 i = 0; i < count; i++) {

			std::string value = "unknown value type";

			switch (itemValueType) {
			case 2u:	// pyeZero; -
				value = "pyeZero";
				break;
			case 3u:	// pyeBool; -
				value = "pyeBool";
				break;
			case 4u:	// pyeInt8; 1 Byte
				value = std::to_string(getInt8(i));
				break;
			case 5u:	// pyeUInt8; 1 Byte
				value = std::to_string(getUInt8(i));
				break;
			case 6u:	// pyeInt16; 2 Byte
				value = std::to_string(getInt16(i));
				break;
			case 7u:	// pyeUInt16; 2 Byte 
				value = std::to_string(getUInt16(i));
				break;
			case 8u:	// pyeInt32; 4 Byte
				value = std::to_string(getInt32(i));
				break;
			case 9u:	// pyeUInt32; 4 Byte 
				value = std::to_string(getUInt32(i));
				break;
			case 14u:	// pyeFloat32; 4 Byte 
				value = std::to_string(getFloat(i));
				break;
			case 10u:	// pyeInt64; 8 Byte
				value = std::to_string(getInt64(i));
				break;
			case 11u:	// pyeUInt64; 8 Byte
				value = std::to_string(getUInt64(i));
				break;
			case 15u:	// pyeFloat64; 8 Byte
				value = std::to_string(getDouble(i));
				break;
			case 12u: {	// pyeInt128; 16 Byte
				int128 memInt128 = getInt128(i);
				std::stringstream ss;
				for (int i = 0; i < memInt128.size(); ++i)
					ss << std::hex << (int)memInt128[i];

				value = ss.str();
			}
			case 13u: {	// pyeUInt128; 16 Byte
				uInt128 memUInt128 = getUInt128(i);
				std::stringstream ss;
				for (int i = 0; i < memUInt128.size(); ++i)
					ss << std::hex << (int)memUInt128[i];

				value = ss.str();
			}
			case 16u: {	// pyeFloat128; 16 Byte
				float128 memFloat128 = getFloat128(i);
				std::stringstream ss;
				for (int i = 0; i < memFloat128.size(); ++i)
					ss << std::hex << (int)memFloat128[i];

				value = ss.str();
			}
			case 17u:	// pyeStringUTF8S; UInt8 as char count
				value = getStringS(i);
				break;
			case 18u:	// pyeStringUTF8L; UInt32 as char count
				value = getStringL(i);
				break;
			case 19u: {	// pyeMemory; UInt32 size of mem
				std::vector<unsigned char> mem = getMemory(i);
				std::stringstream ss;
				for (int i = 0; i < mem.size(); ++i)
					ss << std::hex << (int)mem[i];
				value = ss.str(); 
			}
			}

			data.append("\"");
			data.append(value);
			data.append("\",");
		}

		if (data.back() == ',') {
			std::replace(data.end() - 1, data.end(), ',', ']');
			data.append(",");
		}
		else {
			data.append("]");
		}

		return data;
	}

private:

	virtual void updateObjectHeader();

	virtual unsigned __int64 writeKeyToBuffer(std::vector<unsigned char>& buffer, std::string key) {
		_cntItems++;

		WriteToVector(*getBuffer(), _cntItems, getOffsetValue() + 6 /*pyeValueType(1 byte) + information size of arrayValueType(1 byte) + arraySize(4 byte)*/);

		return buffer.size();
	}

	virtual unsigned __int64 getOffsetItem(unsigned __int32 idx, unsigned __int16 mapRowItem) {
		unsigned __int64 offset = getOffsetValue() + 10/*array header*/;
		pyeValueType typeItems = getArrayDataType();

		switch (typeItems) {
			case pyeValueType::pyeStringUTF8S:
				for (unsigned __int32 i = 0; i < idx; i++) {
					unsigned __int8 stringSizeS;
					ReadFromVector(stringSizeS, (*getBuffer()), offset);
					offset += sizeof(stringSizeS) + stringSizeS;
				}
				break;

			case pyeValueType::pyeStringUTF8L:
			case pyeValueType::pyeMemory:
				for (unsigned __int32 i = 0; i < idx; i++) {
					unsigned __int32 stringSize;
					ReadFromVector(stringSize, (*getBuffer()), offset);
					offset += sizeof(stringSize) + stringSize;
				}
				break;

			default:
				// StreamPos[array value data start] + Index * SizeOf(array data type)
				unsigned __int64 itemSize = getSizeOfFundamentalValueType(typeItems);
				return (offset + (unsigned __int64)idx * itemSize); 
				break;
		}
		
		return offset;
	}
};

/*
* PyeArrayMap
* Name		type				size in byte	usage
* pyeArrayMap	pyeValueType	1				Value=21
* MapLength	UInt16				2				length of the MapStruct; Number of elements of the structure; Max. 65535
* MapStruct	pyeValueType		MapLength		pyeValueType and their order of the structure; 1 Byte per value type
* Size		UInt32				4				size of value data of complete pyeArrayMap
* Count		UInt32				4				count of items
* Item0		structure							Item with structure of pyeValues
* Item1		structure							Item with structure of pyeValues
* Item…		structure							Item with structure of pyeValues
*/

/// <summary>
/// The pyeArrayMap is a container to store items as a map. 
/// Map here describes a structure or record of different value types. 
/// The value types and their order are defined in the header of the pyeArrayMap object. 
/// A structure can contain a maximum of 65535 elements. 
/// All array items use the same map definition.
/// The pyeArrayMap can contain the same value types like the Array object.
/// The header of an pyeArrayMap has a dynamic length, as it depends on the length of the map.
/// </summary>
class PyeArrayMap : public PyeBase<unsigned __int32> {
	/// <summary>
	/// Parental list of this ArrayMap
	/// </summary>
	PyeList* _pLastList = nullptr;

	/// <summary>
	/// Offset of this ArrayMap in the pyeKVS byte stream
	/// </summary>
	unsigned __int64 _offsetObject = 0;

	/// <summary>
	/// Count of items in this ArrayMap
	/// </summary>
	unsigned __int64 _cntItemsAll = 0;

public:
	/// <summary>
	/// Contructor for a pyeArraymap
	/// </summary>
	/// <param name="buffer">pointer to the pyeKVS byte stream</param>
	/// <param name="offsetObjectStart">offset in the byte stream</param>
	PyeArrayMap(std::vector<unsigned char>* buffer, unsigned __int64 offsetObjectStart) {
		setOffsetObject(offsetObjectStart);
		setBuffer(buffer);
	}

	/// <summary>
	/// Contructor for a pyeArraymap
	/// </summary>
	/// <param name="pLastList">parent pyeList of this pyeArrayMap</param>
	/// <param name="mapStruct">pye value type structure of the items in the pyeArrayMap. One value type represents one column</param>
	/// <param name="offsetObjectStart">offset in the byte stream</param>
	PyeArrayMap(PyeList* pLastList, std::vector<pyeValueType> mapStruct, unsigned __int64 offsetObjectStart);

	/// <summary>
	/// Gets the length of the mapStruct (pye value types), Number of elements of the structure; Max. 65535
	/// </summary>
	/// <returns>length</returns>
	unsigned __int16 getMapLength() {
		unsigned __int16 mapLength;
		ReadFromVector(mapLength, *getBuffer(), getOffsetValue() + 1 /*pyeValueType(1 byte)*/);

		return mapLength;
	}

	/// <summary>
	/// Gets the structure of the pyeArrayMap, 1 Byte per value type
	/// </summary>
	/// <returns>vector of pye value types</returns>
	std::vector<pyeValueType> getMapStruct() {
		std::vector<pyeValueType> mapStruct;
		unsigned __int16 mapLength = getMapLength();
		for (unsigned __int64 i = getOffsetValue()+3; i<getOffsetValue()+3+mapLength; i++) {
			pyeValueType pyeType;
			ReadFromVector(pyeType, *getBuffer(), i);
			mapStruct.push_back(pyeType);
		}

		return mapStruct;
	}

	/// <summary>
	/// Gets the size (amount of bytes) of the pyeArraymap object
	/// </summary>
	/// <returns>size</returns>
	virtual unsigned __int32 getSize() {
		unsigned __int16 mapLength = getMapLength();
		unsigned __int32 mapSize;
		ReadFromVector(mapSize, *getBuffer(), getOffsetValue() + 3 /*pyeValueType(1 byte) + information length of map (2 byte)*/ + mapLength);

		return mapSize;
	}

	/// <summary>
	/// Gets the count of items of the pyeArrayMap
	/// </summary>
	/// <returns>count</returns>
	virtual unsigned __int32 getCount() {
		unsigned __int16 mapLength = getMapLength();
		unsigned __int32 mapCount;
		ReadFromVector(mapCount, *getBuffer(), getOffsetValue() + 7 /*pyeValueType(1 byte) + information length of map (2 byte) + mapSize(4 byte)*/ + mapLength);

		return mapCount;
	}

	/// <summary>
	/// Sets the offset of the pyeArrayMap object in the byte stream. 
	/// </summary>
	/// <param name="offset">offset</param>
	virtual void setOffsetObject(unsigned __int64 offset) {
		_offsetObject = offset;
	};

	/// <summary>
	/// Gets the offset of the pyeArrayMap in the byte stream.
	/// </summary>
	/// <returns>offset</returns>
	virtual unsigned __int64 getOffsetObject() {
		return _offsetObject;
	};

	/// <summary>
	/// Generates JSON-formatted string of the pyeArrayMap.
	/// </summary>
	/// <returns>string</returns>
	std::string toStringJSON() {
		std::vector<pyeValueType> mapStruct = getMapStruct();

		unsigned __int32 count = getCount();

		std::string data = "[";

		for (unsigned __int32 i = 0; i < count; i++) {

			data.append("[");

			for (unsigned __int32 j = 0; j < mapStruct.size(); j++) {

				pyeValueType itemValueType = mapStruct[j];

				std::string value = "unknown value type";

				switch (itemValueType) {
				case pyeValueType::pyeZero:
					value = "pyeZero";
					break;
				case pyeValueType::pyeBool:
					value = "pyeBool";
					break;
				case pyeValueType::pyeInt8:
					value = std::to_string(getInt8(i, j));
					break;
				case pyeValueType::pyeUInt8:
					value = std::to_string(getUInt8(i, j));
					break;
				case pyeValueType::pyeInt16:
					value = std::to_string(getInt16(i, j));
					break;
				case pyeValueType::pyeUInt16:
					value = std::to_string(getUInt16(i, j));
					break;
				case pyeValueType::pyeInt32:
					value = std::to_string(getInt32(i, j));
					break;
				case pyeValueType::pyeUInt32:
					value = std::to_string(getUInt32(i, j));
					break;
				case pyeValueType::pyeFloat32:
					value = std::to_string(getFloat(i, j));
					break;
				case pyeValueType::pyeInt64:
					value = std::to_string(getInt64(i, j));
					break;
				case pyeValueType::pyeUInt64:
					value = std::to_string(getUInt64(i, j));
					break;
				case pyeValueType::pyeFloat64:
					value = std::to_string(getDouble(i, j));
					break;
				case pyeValueType::pyeInt128: {
					int128 memInt128 = getInt128(i, j);
					std::stringstream ss;
					for (int k = 0; k < memInt128.size(); ++k)
						ss << std::hex << (int)memInt128[k];

					value = ss.str();
				}
				case pyeValueType::pyeUInt128: {
					uInt128 memUInt128 = getUInt128(i, j);
					std::stringstream ss;
					for (int k = 0; k < memUInt128.size(); ++k)
						ss << std::hex << (int)memUInt128[k];

					value = ss.str();
				}
				case pyeValueType::pyeFloat128: {
					float128 memFloat128 = getFloat128(i, j);
					std::stringstream ss;
					for (int k = 0; k < memFloat128.size(); ++k)
						ss << std::hex << (int)memFloat128[k];

					value = ss.str();
				}
				case pyeValueType::pyeStringUTF8S:
					value = getStringS(i, j);
					break;
				case pyeValueType::pyeStringUTF8L:
					value = getStringL(i, j);
					break;
				case pyeValueType::pyeMemory: {
					std::vector<unsigned char> mem = getMemory(i, j);
					std::stringstream ss;
					for (int k = 0; k < mem.size(); ++k)
						ss << std::hex << (int)mem[k];

					value = ss.str();
				}
				}

				data.append("\"");
				data.append(value);
				data.append("\",");
			}

			if (data.back() == ',') {
				std::replace(data.end() - 1, data.end(), ',', ']');
				data.append(",");
			}
			else {
				data.append("]");
			}
		}

		if (data.back() == ',') {
			std::replace(data.end() - 1, data.end(), ',', ']');
			data.append(",");
		}
		else {
			data.append("]");
		}

		return data;
	}

private:

	virtual void updateObjectHeader();

	virtual unsigned __int64 writeKeyToBuffer(std::vector<unsigned char>& buffer, std::string key) {
		_cntItemsAll++;
		return buffer.size();
	}

	// StreamPos[array value data start] + Index * SizeOf(MapStruct)
	virtual unsigned __int64 getOffsetItem(unsigned __int32 row, unsigned __int16 mapRowItem) {
		unsigned __int64 offset = getOffsetValue() + 11/*array map header*/ + getMapLength();
		std::vector<pyeValueType> mapStruct = getMapStruct();
		unsigned __int64 itemIdx = (unsigned __int64)mapRowItem + (unsigned __int64)row * (unsigned __int64)mapStruct.size();

		for (unsigned __int64 i = 0; i < itemIdx; i++) {
			pyeValueType typeItem = getMapStruct()[i % mapStruct.size()];

			switch (typeItem) {
				case pyeValueType::pyeStringUTF8S:
					unsigned __int8 stringSizeS;
					ReadFromVector(stringSizeS, (*getBuffer()), offset);
					offset += sizeof(stringSizeS) + stringSizeS;
					break;

				case pyeValueType::pyeStringUTF8L:
				case pyeValueType::pyeMemory:
					unsigned __int32 stringSize;
					ReadFromVector(stringSize, (*getBuffer()), offset);
					offset += sizeof(stringSize) + stringSize;
					break;

				default:
					// StreamPos[array value data start] + Index * SizeOf(array data type)
					unsigned __int8 itemSize = getSizeOfFundamentalValueType(typeItem);
					offset += itemSize;
					break;
			}
		}

		return offset;
	}
};

/*
* Name		type			size in byte	usage
* pyeList	pyeValueType	1				Value=1
* Size		UInt32			4				Size of the list in bytes; start after header
* Count		UInt32			4				count of items in the list
* Item0		pyeObject
* Item1		pyeObject
* Item…		pyeObject
*/

/// <summary>
/// The pyeList is a container for further values given by the pyeObject definition. 
/// A pyeList can also contain further sub-lists.
/// The pyeList value type has a fix value header with 4 bytes size in bytes and 4 bytes count of items.
/// This information is strictly speaking duplicated.
/// But it is useful in decoding the stream to perform an internal validation.
/// The size and the count represents only the value data information.
/// The list ends when the number of bytes given by the ‘size’ value have been read / writed.
/// </summary>
class PyeList : public PyeBase<std::string> {
	friend PyeArray;
	friend PyeArrayMap;

	unsigned __int64 _offsetObject = 0;
	std::map<std::string, unsigned __int64> _mapItemIdx;
	PyeList* _pLastList = nullptr;

public:
	PyeList() {}
	PyeList(PyeList* pLastList) {
		_pLastList = pLastList;
	}
	PyeList(std::vector<unsigned char>* buffer, unsigned __int64 offset) {
		setOffsetObject(offset);
		setBuffer(buffer);
		decode();
	}

	PyeArray putArray(std::string key, pyeValueType arrayType) {

		unsigned __int64 offsetObjectStart = writeKeyToBuffer(*getBuffer(), key);

		PyeArray newArray(this, arrayType, offsetObjectStart);

		return newArray;
	}

	PyeArrayMap putArrayMap(std::string key, std::vector<pyeValueType> mapStruct) {

		unsigned __int64 offsetObjectStart = writeKeyToBuffer(*getBuffer(), key);

		PyeArrayMap newArrayMap(this, mapStruct, offsetObjectStart);

		return newArrayMap;
	}

	// create new empty list on buffer
	PyeList putList(std::string key) {

		unsigned __int64 offsetStart = writeKeyToBuffer(*getBuffer(), key);

		// initialize new list object
		PyeList newList(this);
		newList.setBuffer(getBuffer());
		newList.setOffsetObject(offsetStart);

		unsigned __int8 type = pyeValueType::pyeList;
		WriteToVector(*getBuffer(), type, (*getBuffer()).size());

		unsigned __int32 listSize = 0;
		WriteToVector(*getBuffer(), listSize, (*getBuffer()).size());

		unsigned __int32 listCount = 0;
		WriteToVector(*getBuffer(), listCount, (*getBuffer()).size());

		// update size information in header of recent list
		updateObjectHeader();

		// update size information in header of document
		updateHeaderSize();

		return newList;
	}


	PyeArrayMap getArrayMap(std::string key) {
		PyeArrayMap result(getBuffer(), _mapItemIdx[key]);
		return result;
	}

	PyeArray getArray(std::string key) {
		PyeArray result(getBuffer(), _mapItemIdx[key]);
		return result;
	}

	PyeList getList(std::string key) {
		PyeList result(getBuffer(), _mapItemIdx[key]);
		return result;
	}

	virtual void decode() {
		_mapItemIdx.clear();

		unsigned __int64 idx = getOffsetValue() + 1 /*size of pyeValueType*/ + 4 /*size of list size*/ + 4 /*size of list count*/;
		unsigned __int64 listEnd = idx + getSize();
		std::vector<unsigned char>& buffer = *getBuffer();

		while (idx < listEnd) {
			unsigned __int64 idxStart = idx;

			unsigned __int8 keySize;
			ReadFromVector(keySize, buffer, idx);

			idx += sizeof(keySize);

			std::string keyString(&buffer[idx], &buffer[idx] + keySize);

			_mapItemIdx.insert(std::pair<std::string, unsigned __int64>(keyString, idxStart));

			idx += keySize;

			unsigned __int8 valueType;
			ReadFromVector(valueType, buffer, idx);

			idx += sizeof(valueType);

			switch (valueType) {
			case 20u:	// pyeArray; UInt32 Size + UInt32 Count
				idx += 1; /*info data type of array items*/
			case 1u:	// pyeList; UInt32 Size + UInt32 Count
				unsigned __int32 listLength;
				ReadFromVector(listLength, buffer, idx);
				idx += sizeof(listLength);
				idx += 4; /*info count of items*/
				idx += listLength;
				break;
			case 2u:	// pyeZero; -
				// code block
				break;
			case 3u:	// pyeBool; -
				// code block
				break;
			case 4u:	// pyeInt8; 1 Byte
			case 5u:	// pyeUInt8; 1 Byte
				idx += 1;
				break;
			case 6u:	// pyeInt16; 2 Byte
			case 7u:	// pyeUInt16; 2 Byte 
				idx += 2;
				break;
			case 8u:	// pyeInt32; 4 Byte
			case 9u:	// pyeUInt32; 4 Byte 
			case 14u:	// pyeFloat32; 4 Byte 
				idx += 4;
				break;
			case 10u:	// pyeInt64; 8 Byte
			case 11u:	// pyeUInt64; 8 Byte
			case 15u:	// pyeFloat64; 8 Byte
				idx += 8;
				break;
			case 12u:	// pyeInt128; 16 Byte
			case 13u:	// pyeUInt128; 16 Byte
			case 16u:	// pyeFloat128; 16 Byte
				idx += 16;
				break;
			case 17u:	// pyeStringUTF8S; UInt8 as char count
				unsigned __int8 stringLength;
				ReadFromVector(stringLength, buffer, idx);
				idx += sizeof(stringLength);
				idx += stringLength;
				break;
			case 18u:	// pyeStringUTF8L; UInt32 as char count
			case 19u:	// pyeMemory; UInt32 size of mem
				unsigned __int32 dataLength;
				ReadFromVector(dataLength, buffer, idx);
				idx += sizeof(dataLength);
				idx += dataLength;
				break;
			case 21u:	// pyeArrayMap; UInt16 map length + map + UInt32 Size + UInt32 Count
				unsigned __int16 mapLength;
				ReadFromVector(mapLength, buffer, idx);
				idx += sizeof(mapLength);
				idx += mapLength;

				unsigned __int32 mapSize;
				ReadFromVector(mapSize, buffer, idx);
				idx += sizeof(mapSize);
				idx += 4; /*count of items*/
				idx += mapSize;
				break;
			default: // type: 0
				// code block
				break;
			}
		}
	}

	virtual unsigned __int32 getSize() {
		unsigned __int32 listSize;
		unsigned __int64 offset = getOffsetObject() + 1/*info key size*/ + getKeySize() + 1 /*information pyeValueType(1 byte)*/;
		ReadFromVector(listSize, *getBuffer(), offset);

		return listSize;
	}
	virtual unsigned __int32 getCount() {
		unsigned __int32 listCount;
		unsigned __int64 offset = getOffsetObject() + 1/*info key size*/ + getKeySize() + 5 /*information pyeValueType(1 Byte) + listSize(4 byte)*/;
		ReadFromVector(listCount, *getBuffer(), offset);

		return listCount;
	}

	virtual void setOffsetObject(unsigned __int64 offset) {
		_offsetObject = offset;
	};
	virtual unsigned __int64 getOffsetObject() {
		return _offsetObject;
	};

	std::string toStringJSON(std::string separator = "", std::string levelIndicator = "") {
		return toString(separator, levelIndicator, "");
	}

	std::string toStringSimple(std::string separator = "", std::string levelIndicator = "") {
		return toString(separator, levelIndicator, "", false);
	}

private:

	void setLastList(PyeList* pLastList) {
		_pLastList = pLastList;
	}

	PyeList* getLastList() {
		_pLastList;
	}

	virtual void updateObjectHeader() {
		unsigned __int64 offsetFirstItem = getOffsetValue();
		offsetFirstItem += 1; /*information about pye value type (uint8)*/
		offsetFirstItem += 4; /*information about list size (uint32)*/
		offsetFirstItem += 4; /*information about list item count (uint32)*/

		unsigned __int32 listSize = (unsigned __int32)(*getBuffer()).size() - (unsigned __int32)offsetFirstItem;
		WriteToVector(*getBuffer(), listSize, getOffsetValue() + 1 /*information about pye value type*/);

		unsigned __int32 listCount = (unsigned __int32)_mapItemIdx.size();
		WriteToVector(*getBuffer(), listCount, getOffsetValue() + 1 /*information about pye value type*/ + 4 /*information about list size*/);

		if (_pLastList) {
			_pLastList->updateObjectHeader();
		}
	};

	/// @brief 
	/// @param buffer 
	/// @param key 
	/// @return 
	virtual unsigned __int64 writeKeyToBuffer(std::vector<unsigned char>& buffer, std::string key) {
		unsigned __int64 offsetObjectStart = (*getBuffer()).size();

		unsigned __int8 keyLength = key.length();
		WriteToVector(*getBuffer(), keyLength, offsetObjectStart);

		for (int i = 0; i < key.length(); i++) {
			WriteToVector(*getBuffer(), key[i], (*getBuffer()).size());
		}

		_mapItemIdx.insert(std::pair<std::string, unsigned __int64>(key, offsetObjectStart));

		return offsetObjectStart;
	}

	virtual unsigned __int64 getOffsetItem(std::string key, unsigned __int16 mapRowItem) {
		unsigned __int64 idx = _mapItemIdx[key];

		unsigned __int8 keySize;
		ReadFromVector(keySize, *getBuffer(), idx);

		idx += sizeof(keySize); // information about key length (uint8)
		idx += keySize;			// key chars
		idx += 1;				// information pye value type (uint8)

		return idx;
	}

	std::string toString(std::string separator, std::string levelIndicator, std::string levelIndicatorAccu, bool json = true) {
		std::string data = "{";
		data.append(separator);

		int cntItem = 0;
		for (const auto& item : _mapItemIdx) {

			unsigned __int64 offsetItemValueType = getOffsetItem(item.first, NULL);
			offsetItemValueType--; // offset value type

			unsigned __int8 itemValueType;
			ReadFromVector(itemValueType, *getBuffer(), offsetItemValueType);

			data.append(levelIndicatorAccu + levelIndicator);
			data.append("\"");
			data.append(item.first);
			data.append("\"");
			
			if (!json) {
				data.append("(");
				data.append(pyeKVSValueTypeName[itemValueType]);

				if (itemValueType == 1) {		// pyeList;
					data.append(" Count ");
					data.append(std::to_string(getList(item.first).getCount()));
				}
				else if (itemValueType == 20) {	// pyeArray
					data.append(" of ");
					data.append(pyeKVSValueTypeName[getArray(item.first).getArrayDataType()]);
					data.append(" Count ");
					data.append(std::to_string(getArray(item.first).getCount()));
				}
				else if (itemValueType == 21) {	// pyeArrayMap
					std::vector<pyeValueType> types = getArrayMap(item.first).getMapStruct();
					data.append(" of ");
					for (pyeValueType type:types) {
						data.append(pyeKVSValueTypeName[type]);
						data.append(",");
					}
					data = data.substr(0, data.size() - 1);
					data.append(" Count ");
					data.append(std::to_string(getArrayMap(item.first).getCount()));
				}

				data.append(")");
			}
			
			data.append(":");

			std::string value = "unknown value type";

			if (itemValueType == 1) {		// pyeList;
				data.append(getList(item.first).toString(separator, levelIndicator, (levelIndicatorAccu + levelIndicator), json));
			}
			else if (itemValueType == 20) {	// pyeArray
				data.append(getArray(item.first).toStringJSON());
				data.append(separator);
			}
			else if (itemValueType == 21) {	// pyeArrayMap
				data.append(getArrayMap(item.first).toStringJSON());
				data.append(separator);
			}
			else {
				switch (itemValueType) {
				case 1u:
				case 2u:	// pyeZero; -
					value = "pyeZero";
					break;
				case 3u:	// pyeBool; -
					value = "pyeBool";
					break;
				case 4u:	// pyeInt8; 1 Byte
					value = std::to_string(getInt8(item.first));
					break;
				case 5u:	// pyeUInt8; 1 Byte
					value = std::to_string(getUInt8(item.first));
					break;
				case 6u:	// pyeInt16; 2 Byte
					value = std::to_string(getInt16(item.first));
					break;
				case 7u:	// pyeUInt16; 2 Byte 
					value = std::to_string(getUInt16(item.first));
					break;
				case 8u:	// pyeInt32; 4 Byte
					value = std::to_string(getInt32(item.first));
					break;
				case 9u:	// pyeUInt32; 4 Byte 
					value = std::to_string(getUInt32(item.first));
					break;
				case 14u:	// pyeFloat32; 4 Byte 
					value = std::to_string(getFloat(item.first));
					break;
				case 10u:	// pyeInt64; 8 Byte
					value = std::to_string(getInt64(item.first));
					break;
				case 11u:	// pyeUInt64; 8 Byte
					value = std::to_string(getUInt64(item.first));
					break;
				case 15u:	// pyeFloat64; 8 Byte
					value = std::to_string(getDouble(item.first));
					break;
				case 12u: {	// pyeInt128; 16 Byte
					int128 memInt128 = getInt128(item.first);
					std::stringstream ss;
					for (int i = 0; i < memInt128.size(); ++i)
						ss << std::hex << (int)memInt128[i];
						
					value = ss.str();
					}
					break;
				case 13u: {	// pyeUInt128; 16 Byte
					uInt128 memUInt128 = getUInt128(item.first);
					std::stringstream ss;
					for (int i = 0; i < memUInt128.size(); ++i)
						ss << std::hex << (int)memUInt128[i];
						
					value = ss.str();
					}
					break;
				case 16u: {	// pyeFloat128; 16 Byte
					float128 memFloat128 = getFloat128(item.first);
					std::stringstream ss;
					for (int i = 0; i < memFloat128.size(); ++i)
						ss << std::hex << (int)memFloat128[i];

					value = ss.str();
					}
					break;
				case 17u:	// pyeStringUTF8S; UInt8 as char count
					value = getStringS(item.first);
					break;
				case 18u:	// pyeStringUTF8L; UInt32 as char count
					value = getStringL(item.first);
					break;
				case 19u: {	// pyeMemory; UInt32 size of mem
					std::vector<unsigned char> mem = getMemory(item.first);
					std::stringstream ss;
					for (int i = 0; i < mem.size(); ++i)
						ss << std::hex << (int)mem[i];

					value = ss.str();
				}
				}

				data.append("\"");
				data.append(value);
				data.append("\"");

				if (cntItem < _mapItemIdx.size() - 1) {
					data.append(",");
				}

				data.append(separator);
			}

			cntItem++;
		}

		data.append(levelIndicatorAccu);

		data.append("}");

		data.append(separator);

		return data;
	}
};

/*
Name			type	size in byte	usage
StreamPrefix	UInt32	4				constant: $53455950 (PYES)
StreamVersionH	UInt16	2				version high of pyeKVS protocol
StreamVersionL	UInt16	2				version low of pyeKVS protocol
StreamSize		UInt64	8				size of data after the StoreDataHeader; exluded header
*/

/// <summary> 
/// Class to work with pyeKVS data </summary>
class PyeDocument {
	/// <summary> PyeKVS starts with a pyeList. </summary>
	PyeList _rootList;

	/// <summary> 
	/// Length of the header information </summary>
	unsigned __int64 _offsetHeader = 16;

	/// <summary>
	/// This constant value is just to identify the stream as a pyeKVS stream definition. 
	/// It’s also useful to find the start of a pyeKVS document in a data stream. </summary>
	std::string		 _headerPrefix = "PYES";	// __uint32	// = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
	
	/// <summary> 
	/// The StreamVersion high gives the version if the encoding protocol. </summary>
	unsigned __int16 _headerVersionH = 1;		// __uint16	// = buffer[5] << 8 | buffer[4];
	
	/// <summary> 
	/// The StreamVersion low gives the version if the encoding protocol. </summary>
	unsigned __int16 _headerVersionL = 0;		// __uint16	// = buffer[7] << 8 | buffer[6];

	/// <summary> 
	/// The StreamSize gives the numbers of bytes for the complete document but excluded document header. </summary>
	unsigned __int64 _headerStreamSize = 0;			// __uint64	// = buffer[15] << 56 | buffer[14] << 48 | buffer[13] << 40 | buffer[12] << 32 | buffer[11] << 24 | buffer[10] << 16 | buffer[9] << 8 | buffer[8];
	
	/// <summary> 
	/// Name of the pyeKVS root list. default: "" </summary>
	std::string		 _rootName = "";			// __uint8 

public:
	PyeDocument() {

		std::vector<unsigned char> *_pbuffer = new std::vector<unsigned char>(0);
		_rootList.setBuffer(_pbuffer);

		// write header data to buffer
		setHeaderPrefix(_headerPrefix);
		setHeaderVersionH(_headerVersionH);
		setHeaderVersionL(_headerVersionL);
		WriteToVector((*_rootList.getBuffer()), _headerStreamSize, 8);

		// initialisation of root list
		unsigned __int8 rootNameLength = _rootName.length();
		WriteToVector((*_rootList.getBuffer()), rootNameLength, 16);
		/// normally root name = 0
		unsigned __int8 rootType = pyeValueType::pyeList;
		WriteToVector((*_rootList.getBuffer()), rootType, 17);
		unsigned __int32 rootSize = 0;
		WriteToVector((*_rootList.getBuffer()), rootSize, 18);
		unsigned __int32 rootCount = 0;
		WriteToVector((*_rootList.getBuffer()), rootCount, 22);

		// update size information in header of document
		_rootList.updateHeaderSize();

		// set offset to begin of root list object
		_rootList.setOffsetObject(_offsetHeader);
	}

	/// <summary> Constructor of a pyeDocument object with a filename to a pyeKVS file.
	/// </summary>
	/// <param name="filename">Filename of the pyeKVS file</param>
	PyeDocument(std::string filename) {

		// reads data from file in input file stream
		std::ifstream input(filename, std::ios::binary);

		// copies all data into buffer, static to not lost data after loosing scope
		std::vector<unsigned char> *buffer = new std::vector<unsigned char>(std::istreambuf_iterator<char>(input), {});

		// set offset to begin of root list object
		_rootList.setOffsetObject(_offsetHeader);
		// set buffer
		_rootList.setBuffer(buffer);
		// decode buffer
		_rootList.decode();
	}

	/// <summary> Constructor of a pyeDocument object with a pyeKVS buffer.
	/// </summary>
	/// <param name="pbuffer">pyeKVS buffer</param>
	PyeDocument(std::vector<unsigned char>* pbuffer) {
		// set offset to begin of root list object
		_rootList.setOffsetObject(_offsetHeader);
		// set buffer
		_rootList.setBuffer(pbuffer);
		// decode buffer
		_rootList.decode();
	}

	/// <summary> Sets the pointer to a pyeKVS buffer.
	/// </summary>
	/// <param name="pbuffer">pyeKVS buffer</param>
	void setBuffer(std::vector<unsigned char>* pbuffer) {
		// reset offset to root list object
		_rootList.setOffsetObject(_offsetHeader);
		// set new buffer
		_rootList.setBuffer(pbuffer);
	};
	std::vector<unsigned char>* getBuffer() {
		return _rootList.getBuffer();
	};

	/// <summary> Gets the prefix of the header of the pyeDoc.
	/// </summary>
	/// <returns>Low version number</returns>
	unsigned __int32 getHeaderPrefix() {
		unsigned __int32 result;
		ReadFromVector(result, (*_rootList.getBuffer()), 0);
		return result;
	}

	/// <summary> Gets the high version number of the pyeKVS definition.
	/// </summary>
	/// <returns>Low version number</returns>
	unsigned __int16 getHeaderVersionH() {
		unsigned __int32 result;
		ReadFromVector(result, (*_rootList.getBuffer()), 4);
		return result;
	}

	/// <summary> Gets the low version number of the pyeKVS definition.
	/// </summary>
	/// <returns>Low version number</returns>
	unsigned __int16 getHeaderVersionL() {
		unsigned __int32 result;
		ReadFromVector(result, (*_rootList.getBuffer()), 6);
		return result;
	}

	/// <summary> Gets the size of the header of the pyeDoc.
	/// </summary>
	/// <returns>Size of the header</returns>
	unsigned __int64 getHeaderSize() {
		unsigned __int32 result;
		ReadFromVector(result, (*_rootList.getBuffer()), 8);
		return result;
	}

	/// <summary> Sets a prefix in the header of the pyDoc. 
	/// </summary>
	/// <param name="headerPrefix"></param>
	void setHeaderPrefix(std::string headerPrefix) {
		for (int i = 0; i < headerPrefix.length(); i++) {
			WriteToVector((*_rootList.getBuffer()), headerPrefix[i], i);
		}
	}

	/// <summary> Sets the high version number of the pyeKVS definition in the header of the pyeDoc.
	/// </summary>
	/// <param name="value">High version number</param>
	void setHeaderVersionH(unsigned __int16 value) {
		WriteToVector((*_rootList.getBuffer()), value, 4);
	}

	/// <summary> Sets the low version number of the pyeKVS definition in the header of the pyeDoc.
	/// </summary>
	/// <param name="value">Low Version number</param>
	void setHeaderVersionL(unsigned __int16 value) {
		WriteToVector((*_rootList.getBuffer()), value, 6);
	}

	/// <summary> Returns the root list of this pyeDoc.
	/// </summary>
	/// <returns>PyeList</returns>
	PyeList& getRoot() {
		_rootList.setOffsetObject(_offsetHeader);
		return _rootList;
	}

	/// <summary> Returns pyeDocument data as a string with JSON formatting.
	/// </summary>
	/// <param name="separator">Separator between key and value</param>
	/// <param name="levelIndicator">Indicator of the list level</param>
	/// <returns>Data as JSON-formatted string</returns>
	std::string toStringJSON(std::string separator = "", std::string levelIndicator = "") {
		return _rootList.toStringJSON(separator, levelIndicator);
	}

	/// <summary> Returns pyeDocument data as a string with simple formatting.
	/// </summary>
	/// <param name="separator">Separator between key and value</param>
	/// <param name="levelIndicator">Indicator of the list level</param>
	/// <returns>Data as simple formatted string</returns>
	std::string toStringSimple(std::string separator = "", std::string levelIndicator = "") {
		return _rootList.toStringSimple(separator, levelIndicator);
	}
};


/*
* DevArchive


	/*
	pyeValueType	DEC value of pyeValueType	value header											value data		usage
	pyeUnknown		0							no														no				reserved for unknown types
	pyeList			1							UInt32 Size + UInt32 Count								yes				(if size>0)	see notes
	pyeZero			2							no														no				null, nil, Int=0, Float=0, string=”
	pyeBool			3							no														no				BOOL=true; Int=1
	pyeInt8			4							no														1 Byte			little endian; -128 – 127
	pyeUInt8		5							no														1 Byte			little endian; 0 – 255
	pyeInt16		6							no														2 Byte			little endian; -32.768 – 32.767
	pyeUInt16		7							no														2 Byte			little endian; 0 – 65.535
	pyeInt32		8							no														4 Byte			little endian; -2.147.483.648 – 2.147.483.647
	pyeUInt32		9							no														4 Byte			little endian; 0 – 4.294.967.295
	pyeInt64		10							no														8 Byte			little endian; -9.223.372.036.854.775.808 – 9.223.372.036.854.775.807
	pyeUInt64		11							no														8 Byte			little endian; 0 – 18.446.744.073.709.551.615
	pyeInt128		12							no														16 Byte			little endian; -1,70141 E38 – 1,70141E38
	pyeUInt128		13							no														16 Byte			little endian; 0 – 3,40282 E38
	pyeFloat32		14							no														4 Byte			little endian; single
	pyeFloat64		15							no														8 Byte			little endian; double
	pyeFloat128		16							no														16 Byte			little endian; reserve
	pyeStringUTF8S	17							UInt8 as char count										UTF8 chars		Small string; max 255 chars
	pyeStringUTF8L	18							UInt32 as char count									UTF8 chars		Long string; max 4GB
	pyeMemory		19							UInt32 size of mem										mem data bytes	see notes
	pyeArray		20							UInt32 Size + UInt32 Count								values			see notes
	pyeArrayMap		21							UInt16 map length + map + UInt32 Size + UInt32 Count	values			see notes
	*/
/*
	// https://stackoverflow.com/questions/39838716/is-there-a-way-to-call-multiple-functions-on-the-same-object-with-one-line


	// https://stackoverflow.com/questions/35732112/c-tcp-recv-unknown-buffer-size
	// https://www.bfilipek.com/2018/06/any.html
	
	// https://stackoverflow.com/questions/5902360/simplest-way-to-read-binary-data-from-a-stdvectorunsigned-char
	*/


//#endif