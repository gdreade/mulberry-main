/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// Header for CStringOptionsMap class

#ifndef __CSTRINGOPTIONSMAP__MULBERRY__
#define __CSTRINGOPTIONSMAP__MULBERRY__

#include "CBlockOptionsMap.h"

// Classes
class CStringOptionsMap : public CBlockOptionsMap
{

public:
	CStringOptionsMap(void);
	virtual ~CStringOptionsMap(void);

	// Read/Write map
	virtual bool ReadMap(void);
	virtual bool WriteMap(bool verify);
	virtual void DeleteMap(void);

	virtual bool WriteEmptyEnums() const
		{ return false; }

	// Set string
	virtual void SetString(cdstring* buffer)
		{ mBuffer = buffer; }

	// Set upper
	virtual void SetUpper(bool upper)
		{ mUpper = upper; }
	virtual void SetIMSPFormat(bool imsp)
		{ mIMSPFormat = imsp; }

	// Look for values under a key
	virtual bool EnumValue(int num, cdstring& key);
	virtual bool EnumKey(int num, cdstring& key);

	// Section
	virtual void SetSection(const cdstring& section);
	virtual void PushSection(const cdstring& sub_section);

private:
	cdstring* mBuffer;
	bool mUpper;
	bool mIMSPFormat;

	virtual bool WriteKeyValue(const cdstring& key, const cdstring& value);
	virtual bool ReadKeyValue(const cdstring& key, cdstring& value);
	virtual bool RemoveKeyValue(const cdstring& key);
};

#endif
