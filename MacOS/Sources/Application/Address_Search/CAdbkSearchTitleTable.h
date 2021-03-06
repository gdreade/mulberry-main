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


// Header for CAdbkSearchTitleTable class

#ifndef __CADBKSEARCHTITLETABLE__MULBERRY__
#define __CADBKSEARCHTITLETABLE__MULBERRY__

#include "CTitleTable.h"

// Constants

const	ResIDT		MENU_AdbkSearchColumnChanger = 9553;
enum {
	cColumnAddrTitleName = 1,
	cColumnAddrTitleNameLastFirst,
	cColumnAddrTitleNickName,
	cColumnAddrTitleEmail,
	cColumnAddrTitleCompany,
	cColumnAddrTitleAddress,
	cColumnAddrTitlePhoneWork,
	cColumnAddrTitlePhoneHome,
	cColumnAddrTitleFax,
	cColumnAddrTitleURLs,
	cColumnAddrTitleNotes,
	cColumnAddrTitleInsertAfter = cColumnAddrTitleNotes + 2,
	cColumnAddrTitleInsertBefore,
	cColumnAddrTitleDelete
};

// Classes
class	CAdbkSearchTitleTable : public CTitleTable
{
public:
	enum { class_ID = 'AsTi' };

					CAdbkSearchTitleTable();
					CAdbkSearchTitleTable(LStream *inStream);
	virtual 		~CAdbkSearchTitleTable();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	InitColumnChanger(void);				// Init column changer

	virtual void	MenuChoice(short col, bool sort_col, short menu_item);

private:
	static MenuHandle	sColumnChanger;		// Column changer menu
};

#endif
