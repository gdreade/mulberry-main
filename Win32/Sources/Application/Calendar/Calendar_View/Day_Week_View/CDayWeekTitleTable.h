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

#ifndef H_CDayWeekTitleTable
#define H_CDayWeekTitleTable
#pragma once

#include "CSimpleTitleTable.h"

#include "cdstring.h"

class CDayWeekTable;

// ===========================================================================
//	MonthTitleTable

class	CDayWeekTitleTable : public CSimpleTitleTable
{
public:
	enum
	{
		eBroadcast_DblClick = 'DWcl'		// Data: iCal::CICalendarDateTime*
	};


						CDayWeekTitleTable();
	virtual				~CDayWeekTitleTable();
	
	void				SetTable(CDayWeekTable* table)
	{
		mTable = table;
		TableChanged();
	}

	void				TableChanged();

protected:
	CDayWeekTable*				mTable;
	cdstrvect					mTitles;

	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);
	virtual void	DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect);

private:	
			void	GenerateTitles(CDC* pDC);

};

#endif
