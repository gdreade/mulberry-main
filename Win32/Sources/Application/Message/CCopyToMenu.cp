/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CWindowsMenu class


#include "CCopyToMenu.h"

#include "CBrowseMailboxDialog.h"
#include "CDynamicMenu.h"
#include "CIMAPCommon.h"
#include "CMailAccountManager.h"
#include "CMailboxPopup.h"
#include "CMailboxToolbarPopup.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMboxRefList.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

const short cMaxMailbox = 300;

const int cCopyToChoose = 0;
const int cCopyToSingleINBOX = 2;
const int cPopupCopyNone = -1;
const int cPopupCopyChoose = 0;
const int cPopupCopySingleINBOX = 2;
const int cPopupAppendNone = 0;
const int cPopupAppendChoose = 1;
const int cPopupAppendSingleINBOX = 3;

// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// Statics
CCopyToMenu* CCopyToMenu::sMailboxMainMenu = NULL;

CMenu CCopyToMenu::sCopyToMenu;
CMenu CCopyToMenu::sAppendToMenu;
CMenu CCopyToMenu::sCopyToPopupMenu;
ulvector CCopyToMenu::sCopyToPopupServerBreaks;
CMenu CCopyToMenu::sAppendToPopupMenu;
ulvector CCopyToMenu::sAppendToPopupServerBreaks;
bool CCopyToMenu::sCopyToReset = true;
bool CCopyToMenu::sAppendToReset = true;
CCopyToMenu::CMenuList CCopyToMenu::sCopyToMenuList;
CCopyToMenu::CMenuList CCopyToMenu::sAppendToMenuList;
bool CCopyToMenu::sUseCopyToCabinet = false;
bool CCopyToMenu::sUseAppendToCabinet = false;
bool CCopyToMenu::sStripSubscribePrefix = false;
bool CCopyToMenu::sStripFavouritePrefix = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCopyToMenu::CCopyToMenu()
{
	// Get main menu bar menus
	sCopyToMenu.Attach(*CDynamicMenu::GetCopytoSubMenu());
	//sAppendToMenu.Attach(*CDynamicMenu::GetAppendToSubMenu());
	sAppendToMenu.CreatePopupMenu();
	
	// Create popup menus
	sCopyToPopupMenu.LoadMenu(IDR_POPUP_COPYTOMAILBOXES);
	sAppendToPopupMenu.LoadMenu(IDR_POPUP_APPENDTOMAILBOXES);
}

// Default destructor
CCopyToMenu::~CCopyToMenu()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Respond to list changes
void CCopyToMenu::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMailAccountManager::eBroadcast_BeginMailAccountChange:
	case CMboxProtocol::eBroadcast_BeginListChange:
	case CMboxProtocol::eBroadcast_BeginListUpdate:
		break;
	case CMailAccountManager::eBroadcast_EndMailAccountChange:
	case CMboxProtocol::eBroadcast_EndListChange:
	case CMboxProtocol::eBroadcast_EndListUpdate:
		break;

	case CMailAccountManager::eBroadcast_MRUCopyToChange:
		if (sUseCopyToCabinet)
		{
			CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteCopyTo);
			ResetFavourite(&sCopyToMenu, sCopyToPopupMenu.GetSubMenu(0), sCopyToPopupServerBreaks, *list,
							CMailAccountManager::sMailAccountManager->GetMRUCopyTo(),
							IDM_CopyToMailboxChoose, IDM_CopyToPopupChoose, true);
			CMailboxPopup::ResetMenuList();
			CMailboxToolbarPopup::ResetMenuList();
		}
		break;

	case CMailAccountManager::eBroadcast_MRUAppendToChange:
		if (sUseAppendToCabinet)
		{
			CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteAppendTo);
			ResetFavourite(&sAppendToMenu, sAppendToPopupMenu.GetSubMenu(0), sAppendToPopupServerBreaks, *list,
							CMailAccountManager::sMailAccountManager->GetMRUAppendTo(),
							IDM_AppendToMailboxChoose, IDM_AppendToPopupNone, false);
			CMailboxPopup::ResetMenuList();
			CMailboxToolbarPopup::ResetMenuList();
		}
		break;

	case CMailAccountManager::eBroadcast_NewMailAccount:
	case CMailAccountManager::eBroadcast_RemoveMailAccount:
		sCopyToReset = true;
		sAppendToReset = true;
		break;
	case CMboxProtocol::eBroadcast_NewList:
	case CMboxProtocol::eBroadcast_RemoveList:
	case CTreeNodeList::eBroadcast_ResetList:
		ChangedList(static_cast<CTreeNodeList*>(param));
		break;
	case CTreeNodeList::eBroadcast_AddNode:
	case CTreeNodeList::eBroadcast_ChangeNode:
	case CTreeNodeList::eBroadcast_DeleteNode:
		CTreeNodeList* aList = NULL;
		switch(msg)
		{
			case CTreeNodeList::eBroadcast_AddNode:
				aList = static_cast<CTreeNodeList::SBroadcastAddNode*>(param)->mList;
				break;
			case CTreeNodeList::eBroadcast_ChangeNode:
				aList = static_cast<CTreeNodeList::SBroadcastChangeNode*>(param)->mList;
				break;
			case CTreeNodeList::eBroadcast_DeleteNode:
				aList = static_cast<CTreeNodeList::SBroadcastDeleteNode*>(param)->mList;
				break;
		}
		ChangedList(aList);
		break;
	}
}

void CCopyToMenu::ChangedList(CTreeNodeList* list)
{
	if (typeid(*static_cast<CTreeNodeList*>(list)) == typeid(CMboxList))
	{
		// Only reset if not favourites
		if (!sUseCopyToCabinet)
			sCopyToReset = true;
		if (!sUseAppendToCabinet)
			sAppendToReset = true;

		// Must tell popups of change too
		CMailboxPopup::ChangeMenuList();
		CMailboxToolbarPopup::ChangeMenuList();
	}
	else if (typeid(*static_cast<CTreeNodeList*>(list)) == typeid(CMboxRefList))
	{
		// Only reset if favourites
		if (sUseCopyToCabinet &&
			CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteCopyTo)
			sCopyToReset = true;
		if (sUseAppendToCabinet &&
			CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteAppendTo)
			sAppendToReset = true;
	}
}

void CCopyToMenu::SetMenuOptions(bool use_copyto_cabinet, bool use_appendto_cabinet)
{
	// Look for change in state
	if ((sUseCopyToCabinet != use_copyto_cabinet) ||
		(sUseAppendToCabinet != use_appendto_cabinet))
		DirtyMenuList();

	sUseCopyToCabinet = use_copyto_cabinet;
	sUseAppendToCabinet = use_appendto_cabinet;
}

// Set the menu items from the various lists
void CCopyToMenu::ResetMenuList()
{
	// Only do if required
	if (!sCopyToReset && !sAppendToReset)
		return;

	// Always delete existing menu arrays
	if (sCopyToMenuList.size() && sCopyToReset)
	{
		// Delete all submenus
		for(CMenuList::iterator iter = sCopyToMenuList.begin(); iter != sCopyToMenuList.end(); iter++)
			delete *iter;
		sCopyToMenuList.clear();
	}
	
	// Always delete existing menu arrays
	if (sAppendToMenuList.size() && sAppendToReset)
	{
		// Delete all submenus
		for(CMenuList::iterator iter = sAppendToMenuList.begin(); iter != sAppendToMenuList.end(); iter++)
			delete *iter;
		sAppendToMenuList.clear();
	}
	
	// Can only do this is accounts already setup
	if (CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->GetProtocolCount())
	{
		if (sCopyToReset)
		{
			if (sUseCopyToCabinet)
			{
				CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteCopyTo);
				ResetFavourite(&sCopyToMenu, sCopyToPopupMenu.GetSubMenu(0), sCopyToPopupServerBreaks, *list,
								CMailAccountManager::sMailAccountManager->GetMRUCopyTo(),
								IDM_CopyToMailboxChoose, IDM_CopyToPopupChoose, true);
			}
			else
				ResetAll(&sCopyToMenu, sCopyToPopupMenu.GetSubMenu(0), sCopyToPopupServerBreaks,
								IDM_CopyToMailboxChoose, IDM_CopyToPopupChoose, true);
		}

		if (sAppendToReset)
		{
			if (sUseAppendToCabinet)
			{
				CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteAppendTo);
				ResetFavourite(&sAppendToMenu, sAppendToPopupMenu.GetSubMenu(0), sAppendToPopupServerBreaks, *list,
								CMailAccountManager::sMailAccountManager->GetMRUAppendTo(),
								IDM_AppendToMailboxChoose, IDM_AppendToPopupNone, false);
			}
			else
				ResetAll(&sAppendToMenu, sAppendToPopupMenu.GetSubMenu(0), sAppendToPopupServerBreaks,
								IDM_AppendToMailboxChoose, IDM_AppendToPopupNone, false);
		}

		// Always resync popups after change
		CMailboxPopup::ResetMenuList();
		CMailboxToolbarPopup::ResetMenuList();
	}
	
	// Always do this last as resolve may force reset
	sCopyToReset = false;
	sAppendToReset = false;
}

bool CCopyToMenu::TestPrefix(const CMboxList& list, const cdstring& offset)
{
	// Now add current items
	size_t offset_length = offset.length();
	for(CMboxList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (!(*iter)->IsDirectory())
		{
			cdstring theName = (*iter)->GetName();
			if (offset_length && ::strncmp(offset.c_str(), theName.c_str(), offset_length))
				return false;
		}
	}
	
	return true;
}

// Add entire protocol to menu
void CCopyToMenu::AddProtocolToMenu(CMboxProtocol* proto, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, bool single)
{
	// Count number in list
	unsigned long num_mbox = proto->CountHierarchy();
	bool need_separator = false;

	// Add INBOX if present
	if (proto->GetINBOX())
	{
		cdstring theName = (acct_name ? proto->GetINBOX()->GetAccountName() : proto->GetINBOX()->GetName());
		CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(menu_row, false), pos++ + offset, theName.c_str());
		need_separator = true;
	}

	const CHierarchies& hiers = proto->GetHierarchies();

	// Check to see whether flat wd is required
	cdstring name_offset;
	if (single && proto->FlatWD() && hiers.at(1)->IsRootName(hiers.at(1)->GetDirDelim()))
	{
		name_offset = hiers.at(1)->GetRootName();

		// Check to see whether subscribed is all from same root
		if (TestPrefix(*hiers.front(), name_offset))
			sStripSubscribePrefix = true;
		else
		{
			name_offset = cdstring::null_str;
			sStripSubscribePrefix = false;
		}
	}

	// Always add subscribed
	AppendListToMenu(*hiers.front(), menu, menu_row, pos, offset, acct_name, name_offset);
	if (hiers.front()->size())
		need_separator = true;

	// Limit number in list
	if (num_mbox + pos > cMaxMailbox)
	{
		// Add 'Too Many Mailboxes' item (disabled)
		cdstring str;
		str.FromResource(IDE_TooManyForMenu);
		if (need_separator)
		{
			CUnicodeUtils::AppendMenuUTF8(menu, ::AppendMenuFlags(menu_row, true));
			pos++;
		}
		CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(menu_row, false) | MF_DISABLED, 0, str);
		pos++;
		
		// Special - if any default copy to is on this server then add it as a special case
		size_t name_offset_length = name_offset.length();
		const CIdentityList& ids = CPreferences::sPrefs->mIdentities.GetValue();
		cdstrvect added;
		for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++)
		{
			cdstring copyto = (*iter).GetCopyTo(true);
			if (!(*iter).GetCopyToNone(true) && !(*iter).GetCopyToChoose(true))
			{
				// Must not be duplicate
				cdstrvect::const_iterator found = std::find(added.begin(), added.end(), copyto);
				if (found != added.end())
					continue;

				// Check for account match
				cdstring acctname = proto->GetAccountName();
				acctname += cMailAccountSeparator;
				if (!::strncmp(acctname, copyto, acctname.length()))
				{
					// Punt over account prefix if not required
					cdstring use_name;
					if (acct_name)
						use_name = copyto;
					else
						use_name = copyto.c_str() + acctname.length();

					if (name_offset_length && (::strncmp(name_offset.c_str(), use_name.c_str(), name_offset_length) == 0))
						CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(menu_row, false), pos++ + offset, use_name.c_str() + name_offset_length);
					else
						CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(menu_row, false), pos++ + offset, use_name.c_str());
					added.push_back(copyto);
				}
			}
		}
	}
	else
	{
		// Add remainder (with separators)
		for(CHierarchies::const_iterator iter = hiers.begin() + 1; iter != hiers.end(); iter++)
		{
			// Only bother if something present
			if ((*iter)->size())
			{
				// Do first separator only if its needed
				if (need_separator)
				{
					CUnicodeUtils::AppendMenuUTF8(menu, ::AppendMenuFlags(menu_row, true));
					pos++;
				}
				else
					need_separator = true;
				AppendListToMenu(**iter, menu, menu_row, pos, offset, acct_name, name_offset);
			}
		}
	}
}

// Append entire list to menu
void CCopyToMenu::AppendListToMenu(const CMboxList& list, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, const cdstring& noffset)
{
	// Now add current items
	size_t noffset_length = noffset.length();
	for(CMboxList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (!(*iter)->IsDirectory())
			AppendMbox(*iter, menu, menu_row, pos, offset, acct_name, noffset, noffset_length);
	}
}

// Append entire list to menu
void CCopyToMenu::AppendListToMenu(const CMboxRefList& list, bool dynamic, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, const cdstring& noffset, bool reverse)
{
	// Now add current items
	size_t noffset_length = noffset.length();
	for(int i = 0 ; i < list.size(); i++)
	{
		CMboxRefList::const_iterator iter1 = reverse ? list.end() - 1 - i : list.begin() + i;

		// Look for wildcard items and resolve all of them
		if ((*iter1)->IsWildcard())
		{
			// Resolve to mboxes - maybe wildcard so get list
			CMboxList match;
			static_cast<CWildcardMboxRef*>(*iter1)->ResolveMbox(match, dynamic);

			// Add each mailbox in resolved list
			for(CMboxList::iterator iter2 = match.begin(); iter2 != match.end(); iter2++)
			{
				// Do not add directories
				if (!(*iter2)->IsDirectory())
					AppendMbox(*iter2, menu, menu_row, pos, offset, acct_name, noffset, noffset_length);
			}
		}

		// Do not add directories
		else if (!(*iter1)->IsDirectory())
			AppendMbox(*iter1, menu, menu_row, pos, offset, acct_name, noffset, noffset_length);
	}
}

void CCopyToMenu::AppendMbox(const CTreeNode* mbox, CMenu* menu, int& menu_row, int& pos, int offset, bool acct_name, const cdstring& noffset, size_t noffset_length)
{
	cdstring theName = mbox->GetAccountName(acct_name);
	if (noffset_length && (::strncmp(noffset.c_str(), theName.c_str(), noffset_length) == 0))
		CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(menu_row, false), pos++ + offset, theName.c_str() + noffset_length);
	else
		CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(menu_row, false), pos++ + offset, theName.c_str());
}

// Find mailbox from menu
bool CCopyToMenu::GetMbox(bool copy_to, int item_num, CMbox*& found)
{
	// Look for browse
	if (item_num == cCopyToChoose)
	{
		found = NULL;
		bool ignore = false;
		return CBrowseMailboxDialog::PoseDialog(false, false, found, ignore);
	}

	// Check for favourite
	bool favourite = ((copy_to && sUseCopyToCabinet) ||
						(!copy_to && sUseAppendToCabinet));
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	bool multi = favourite ? (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1) : false;

	// Determine server index & get menu handle
	unsigned long server_index = 0;
	CMenu* menu = copy_to ? &sCopyToMenu : &sAppendToMenu;
	if (!favourite && (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1))
	{
		// Adjust for offset due to Choose item
		item_num -= 2;

		CMenuList& menu_list = copy_to ? sCopyToMenuList : sAppendToMenuList;
		for(CMenuList::iterator iter = menu_list.begin(); iter != menu_list.end(); iter++, server_index++)
		{
			if (item_num > (*iter)->GetMenuItemCount() - 1)
				item_num -= (*iter)->GetMenuItemCount();
			else
			{
				menu = *iter;
				break;
			}
		}
	}

	// May need to add back in WD prefix
	cdstring mbox_name;
	if ((protos.size() == 1) && protos.front()->FlatWD() &&
		(favourite ? sStripFavouritePrefix : sStripSubscribePrefix))
		mbox_name = protos.front()->GetHierarchies().at(1)->GetRootName();

	// Get name
	cdstring theTxt = CUnicodeUtils::GetMenuStringUTF8(menu, item_num, MF_BYPOSITION);

	// Process any prefix - not for INBOX
	if (theTxt == cINBOX)
		mbox_name = theTxt;
	else
		mbox_name += theTxt;

	
	if (!multi)
		mbox_name = protos.at(server_index)->GetAccountName() + cMailAccountSeparator + mbox_name;

	// Create mbox ref
	CMboxRef ref(mbox_name, '.');
	found = ref.ResolveMbox(true);
	
	return found != NULL;
}

bool CCopyToMenu::GetPopupMboxSend(bool copy_to, short item_num, CMbox*& found, bool& set_as_default)
{
	return GetPopupMbox(copy_to, item_num, true, true, found, set_as_default);
}

bool CCopyToMenu::GetPopupMbox(bool copy_to, short item_num, CMbox*& found, bool do_choice)
{
	bool ignore = false;
	return GetPopupMbox(copy_to, item_num, do_choice, false, found, ignore);
}

bool CCopyToMenu::GetPopupMbox(bool copy_to, short item_num, bool do_choice, bool sending, CMbox*& found, bool& set_as_default)
{
	// Return false only if a choice was requested but cancelled

	// Special for first items
	if (item_num == (copy_to ? cPopupCopyNone : cPopupAppendNone))
	{
		found = (CMbox*) -1;
		return true;
	}
	else if (item_num == (copy_to ? cPopupCopyChoose : cPopupAppendChoose))
	{
		found = NULL;
		if (do_choice)
			CBrowseMailboxDialog::PoseDialog(false, sending, found, set_as_default);

		return (found != NULL) || !do_choice;
	}
	
	CMenu* menu = copy_to ? sCopyToPopupMenu.GetSubMenu(0) : sAppendToPopupMenu.GetSubMenu(0);
	unsigned long server_index = 0;
	bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
	bool use_cabinet = (copy_to && sUseCopyToCabinet) || (!copy_to && sUseAppendToCabinet);
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();

	// May need to add back in WD prefix
	cdstring mbox_name;
	if ((protos.size() == 1) && protos.front()->FlatWD() &&
		(use_cabinet ? sStripFavouritePrefix : sStripSubscribePrefix))
		mbox_name = protos.front()->GetHierarchies().at(1)->GetRootName();

	// Get name
	cdstring theTxt = CUnicodeUtils::GetMenuStringUTF8(menu, item_num, MF_BYPOSITION);
	
	// Process any prefix - not for INBOX
	if (theTxt == cINBOX)
		mbox_name = theTxt;
	else
		mbox_name += theTxt;

	// If not cabinet, need to take server breaks into account
	if (!multi)
		mbox_name = protos.front()->GetAccountName() + cMailAccountSeparator + mbox_name;

	// Create mbox ref
	CMboxRef ref(mbox_name, '.');
	found = ref.ResolveMbox(true);
		
	return found != NULL;
}

bool CCopyToMenu::GetPopupMboxName(bool copy_to, int item_num, cdstring& found, bool do_choice)
{
	CMbox* mbox = NULL;
	if (GetPopupMbox(copy_to, item_num, mbox, do_choice))
	{
		if (mbox && (mbox != (CMbox*) -1))
		{
			//bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1);
			bool multi = true;

			found = (multi ? mbox->GetAccountName() : mbox->GetName());
		}
		else if (mbox)
			found = cdstring::null_str;
		else
			found = "\1";
		
		return true;
	}
	else
		return false;
}

// Find position of mailbox in menu
SInt16 CCopyToMenu::FindPopupMboxPos(bool copy_to, const char* name)
{
	// Check for 'Choose' special
	if ((*name == 1) && (*(name+1) == 0))
		return (copy_to ? cPopupCopyChoose : cPopupAppendChoose);

	// Loop over all items
	CMenu* menu = copy_to ? sCopyToPopupMenu.GetSubMenu(0) : sAppendToPopupMenu.GetSubMenu(0);
	int max_items = menu->GetMenuItemCount();
	for(int i = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX; i < max_items; i++)
	{
		// Check for disabled menu
		if (menu->GetMenuState(i, MF_BYPOSITION) & MF_DISABLED)
			continue;

		// See if name matches
		cdstring found;
		if (GetPopupMboxName(copy_to, i, found) && (found == name))
			return i;
	}

	// Nothing found
	return 0;
}


// Set the menu items from the various lists
void CCopyToMenu::ResetAll(CMenu* main_menu, CMenu* popup_menu, ulvector& breaks, int main_offset, int popup_offset, bool copy_to)
{
	// Remove any existing items from main menu
	int num_menu = main_menu->GetMenuItemCount();
	for(int i = cCopyToSingleINBOX; i < num_menu; i++)
		main_menu->RemoveMenu(cCopyToSingleINBOX, MF_BYPOSITION);
	
	// Remove any existing items from popup menu
	num_menu = popup_menu->GetMenuItemCount();
	for(int i = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX; i < num_menu; i++)
		popup_menu->RemoveMenu(copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX, MF_BYPOSITION);
	breaks.clear();

	int pos_main = cCopyToSingleINBOX;
	int pos_popup = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX;

	int row_main = pos_main;
	int row_popup = pos_popup;

	// Only do the reset if mail accounts sets up
	if (!CMailAccountManager::sMailAccountManager)
		return;

	// Look for single or multiple accounts
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	if (CMailAccountManager::sMailAccountManager->GetProtocolCount() == 1)
	{
		// Single account - add to main menu
		AddProtocolToMenu(protos.front(), main_menu, row_main, pos_main, main_offset, false, true);
		AddProtocolToMenu(protos.front(), popup_menu, row_popup, pos_popup, popup_offset, false, true);
	}
	else if (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1)
	{
		bool first = true;
		CMenuList& menu_list = copy_to ? sCopyToMenuList : sAppendToMenuList;
		
		// Add each protocol to menu as sub-menu
		for(CMboxProtocolList::const_iterator iter = protos.begin(); iter != protos.end(); iter++)
		{
			CMenu* menu = new CMenu;
			menu->CreatePopupMenu();
			menu_list.push_back(menu);
			
			// Add protocol to menu
			int sub_row = 0;
			AddProtocolToMenu(*iter, menu, sub_row, pos_main, main_offset, false);

			// Add this as sub-menu to main
			cdstring theName = (*iter)->GetAccountName();
			CUnicodeUtils::AppendMenuUTF8(menu, MF_STRING | ::AppendMenuFlags(row_main, false) | MF_POPUP, (UINT) menu->m_hMenu, theName.c_str());
			pos_main++;

			// Add this as sub-menu to popup
			if (!first)
			{
				popup_menu->AppendMenu(::AppendMenuFlags(row_popup, true));
				pos_popup++;
			}
			else
				first = false;
			breaks.push_back(pos_popup);
			AddProtocolToMenu(*iter, popup_menu, row_popup, pos_popup, popup_offset, true);
		}
	}
	
	// Force reset of popups
	CMailboxPopup::ResetMenuList();
	CMailboxToolbarPopup::ResetMenuList();
}


// Set the menu items from the various lists
void CCopyToMenu::ResetFavourite(CMenu* main_menu, CMenu* popup_menu, ulvector& breaks,
									const CMboxRefList& items, const CMboxRefList& mrus,
									int main_offset, int popup_offset, bool copy_to)
{
	// Remove any existing items from main menu
	int num_menu = main_menu->GetMenuItemCount();
	for(int i = cCopyToSingleINBOX; i < num_menu; i++)
		main_menu->RemoveMenu(cCopyToSingleINBOX, MF_BYPOSITION);
	
	// Remove any existing items from popup menu
	num_menu = popup_menu->GetMenuItemCount();
	for(int i = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX; i < num_menu; i++)
		popup_menu->RemoveMenu(copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX, MF_BYPOSITION);
	breaks.clear();

	int pos_main = cCopyToSingleINBOX;
	int pos_popup = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX;

	int row_main = pos_main;
	int row_popup = pos_popup;

	// Only do the reset if mail accounts sets up
	if (!CMailAccountManager::sMailAccountManager)
		return;

	bool acct_name = (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);

	// Check to see whether flat wd is required
	cdstring name_offset;
	sStripFavouritePrefix = false;
	if (!acct_name)
	{
		const CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocolList().front();
		const CHierarchies& hiers = proto->GetHierarchies();
		if (proto->FlatWD() && hiers.at(1)->IsRootName(hiers.at(1)->GetDirDelim()))
		{
			name_offset = hiers.at(1)->GetRootName();

			// Check to see whether subscribed is all from same root
			if (TestPrefix(*hiers.front(), name_offset))
				sStripFavouritePrefix = true;
			else
				name_offset = cdstring::null_str;
		}
	}

	// Now add current items
	AppendListToMenu(items,true,  main_menu, row_main, pos_main, main_offset, acct_name, name_offset);
	AppendListToMenu(items, false, popup_menu, row_popup, pos_popup, popup_offset, acct_name, name_offset);
	
	// Add MRU break
	CUnicodeUtils::AppendMenuUTF8(main_menu, ::AppendMenuFlags(row_main, true));
	pos_main++;
	CUnicodeUtils::AppendMenuUTF8(popup_menu, ::AppendMenuFlags(row_popup, true));
	pos_popup++;

	// Add MRUs (in reverse)
	AppendListToMenu(mrus, true, main_menu, row_main, pos_main, main_offset, acct_name, name_offset, true);
	AppendListToMenu(mrus, false, popup_menu, row_popup, pos_popup, popup_offset, acct_name, name_offset, true);
}
