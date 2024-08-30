//#include "pch.h"

#include <vector>
#include <map>
#include <tuple>
#include <array>

#include "pyeKVS.h"


unsigned __int32 getSizeOfAdvancedValueType(std::vector<unsigned __int8>& buffer, unsigned __int64 offset, unsigned __int8 valueType) 
{
	switch (valueType) {
	case 17u:	// pyeStringUTF8S; UInt8 as char count
		unsigned __int8 stringLength;
		ReadFromVector(stringLength, buffer, offset);
		return stringLength;
	case 18u:	// pyeStringUTF8L; UInt32 as char count
	case 19u:	// pyeMemory; UInt32 size of mem
		unsigned __int32 dataLength;
		ReadFromVector(dataLength, buffer, offset);
		return stringLength;
	default: // type: 0
		return -1;
	}
};

unsigned __int8 getSizeOfFundamentalValueType(pyeValueType valueType) {
	switch (valueType) {
	case pyeValueType::pyeZero:		// pyeZero; -
		return -1;
	case pyeValueType::pyeBool:		// pyeBool; -
		return -1;
	case pyeValueType::pyeInt8:		// pyeInt8; 1 Byte
	case pyeValueType::pyeUInt8:	// pyeUInt8; 1 Byte
		return 1;
	case pyeValueType::pyeInt16:	// pyeInt16; 2 Byte
	case pyeValueType::pyeUInt16:	// pyeUInt16; 2 Byte 
		return 2;
	case pyeValueType::pyeInt32:	// pyeInt32; 4 Byte
	case pyeValueType::pyeUInt32:	// pyeUInt32; 4 Byte 
	case pyeValueType::pyeFloat32:	// pyeFloat32; 4 Byte 
		return 4;
	case pyeValueType::pyeInt64:	// pyeInt64; 8 Byte
	case pyeValueType::pyeUInt64:	// pyeUInt64; 8 Byte
	case pyeValueType::pyeFloat64:	// pyeFloat64; 8 Byte
		return 8;
	case pyeValueType::pyeInt128:	// pyeInt128; 16 Byte
	case pyeValueType::pyeUInt128:	// pyeUInt128; 16 Byte
	case pyeValueType::pyeFloat128:	// pyeFloat128; 16 Byte
		return 16;
	default: // type: 0
		return -1;
	}
};

PyeArray::PyeArray(PyeList* pLastList, pyeValueType arrayType, unsigned __int64 offsetObjectStart) {
	_pLastList = pLastList;
	setBuffer(_pLastList->getBuffer());
	setOffsetObject(offsetObjectStart);

	unsigned __int8 _type = pyeValueType::pyeArray;
	WriteToVector(*getBuffer(), _type, (*getBuffer()).size());

	unsigned __int8 _arrayType = arrayType;
	WriteToVector(*getBuffer(), _arrayType, (*getBuffer()).size());

	unsigned __int32 _arraySize = 0;
	WriteToVector(*getBuffer(), _arraySize, (*getBuffer()).size());

	unsigned __int32 _arrayCount = 0;
	WriteToVector(*getBuffer(), _arrayCount, (*getBuffer()).size());

	// update size information in header of recent list
	updateObjectHeader();

	// update size information in header of document
	updateHeaderSize();
}

void PyeArray::updateObjectHeader() {
	unsigned __int64 offsetFirstItem = getOffsetValue();
	offsetFirstItem += 1; /* information about pye value type (uint8) */
	offsetFirstItem += 1; /* information about pye array type (uint8) */
	offsetFirstItem += 4; /* information about array size (uint32) */
	offsetFirstItem += 4; /* information about array item count (uint32) */

	unsigned __int32 arraySize = (unsigned __int32)getBuffer()->size() - (unsigned __int32)offsetFirstItem;
	WriteToVector(*getBuffer(), arraySize, getOffsetValue() + 1 /*pye value type*/ + 1 /*pye array type*/);

	WriteToVector(*getBuffer(), _cntItems, getOffsetValue() + 1 /*information about pye value type*/ + 1 /*pye array type*/ + 4 /*list size*/);

	if (_pLastList) {
		_pLastList->updateObjectHeader();
	}
}

/* PyeArrayMap
Name			type			size in byte	usage
pyeArrayMap		pyeValueType	1				Value=21
MapLength		UInt16			2				length of the MapStruct; Number of elements of the structure; Max. 65535
MapStruct		pyeValueType	MapLength		pyeValueType and their order of the structure; 1 Byte per value type
Size			UInt32			4				size of value data of complete pyeArrayMap
Count			UInt32			4				count of items
*/
PyeArrayMap::PyeArrayMap(PyeList* pLastList, std::vector<pyeValueType> mapStruct, unsigned __int64 offsetObjectStart) {
	_pLastList = pLastList;
	setBuffer(_pLastList->getBuffer());
	setOffsetObject(offsetObjectStart);

	unsigned __int8 _type = pyeValueType::pyeArrayMap;
	WriteToVector(*getBuffer(), _type, (*getBuffer()).size());

	unsigned __int16 mapLength = mapStruct.size();
	WriteToVector(*getBuffer(), mapLength, (*getBuffer()).size());

	for (unsigned __int8 type : mapStruct) {
		WriteToVector(*getBuffer(), type, (*getBuffer()).size());
	}

	unsigned __int32 _mapSize = 0;
	WriteToVector(*getBuffer(), _mapSize, (*getBuffer()).size());

	unsigned __int32 _mapCount = 0;
	WriteToVector(*getBuffer(), _mapCount, (*getBuffer()).size());

	// update size information in header of recent list
	updateObjectHeader();

	// update size information in header of document
	updateHeaderSize();
}

void PyeArrayMap::updateObjectHeader() {
	unsigned __int16 mapLength = getMapLength();

	unsigned __int64 offsetFirstItem = getOffsetValue();
	offsetFirstItem += 1; /* information about pye value type (uint8) */
	offsetFirstItem += 2; /* information about pye map length (uint16) */
	offsetFirstItem += mapLength;
	offsetFirstItem += 4; /* information about array size (uint32) */
	offsetFirstItem += 4; /* information about array item count (uint32) */

	unsigned __int32 mapSize = (unsigned __int32)getBuffer()->size() - (unsigned __int32)offsetFirstItem;
	WriteToVector(*getBuffer(), mapSize, getOffsetValue() + 1 /*pye value type*/ + 2 /*pye map length*/ + mapLength);

	unsigned __int32 cntItems = 0;
	cntItems = (unsigned __int32)(_cntItemsAll / mapLength); // count only full/complete items
	WriteToVector(*getBuffer(), cntItems, getOffsetValue() + 7 /*pyeValueType(1 byte) + information length of map (2 byte) + mapSize(4 byte)*/ + mapLength);

	if (_pLastList) {
		_pLastList->updateObjectHeader();
	}
}