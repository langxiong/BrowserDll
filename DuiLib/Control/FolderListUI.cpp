#include "stdafx.h"
#include <shlobj.h>
//#include "RcCommon.h"
#include "FolderListUI.h"
#include <process.h>
#include <algorithm>

using namespace std;
const WCHAR *picFMT[30] = 
{
	_T(".bmp"),_T(".cut"),_T(".dds"),_T(".gif"),_T(".hdr"),_T(".ico"),_T(".iff"),_T(".jng"),
	_T(".jpeg"),_T(".jpg"),_T(".jif"),_T(".jpe"),_T(".koa"),_T(".mng"),_T(".pbm"),_T(".pcd"),
	_T(".pcx"),_T(".pgm"),_T(".png"),_T(".ppm"),_T(".psd"),_T(".ras"),_T(".tga"),_T(".targa"),
	_T(".tif"),_T(".tiff"),_T(".wbmp"),_T(".xbm"),_T(".xpm"),_T(".lbm")
};
CFolderListUI::CFolderListUI(): root(NULL)
{
	SetItemShowHtml(true);
	root = new CFListNode;
	root->nLevel = -1;
	root->bChildVisble = true;
	root->pListElement = NULL;
	m_ListInfo.dwHotBkColor = 0xFFACD0FF;
	m_ListInfo.dwSelectedBkColor = 0xFFDBE8F9;
	m_lastSelectIndex = 0;
//	m_imgLoadMgr = CPMImageLoadMgr::GetInstance();
}
void CFolderListUI::Init()
{
	m_pManager->AddNotifier(this);
	CListUI::Init();
	CFListNode *pNode;
	pNode = AddFolder(_T("桌面"), CSIDL_DESKTOP, FOLDER_TYPE_DESKTOP, NULL);
	pNode = AddFolder(_T("我的文档"), CSIDL_MYDOCUMENTS, FOLDER_TYPE_MYDOCS, NULL);
	SetChildVisible(pNode,false);
	pNode = AddFolder(_T("图片收藏"), CSIDL_MYPICTURES, FOLDER_TYPE_MYPICS, NULL);
	SetChildVisible(pNode,false);
	SelectItem(0);
	CListLabelElementUI* pListElement = (CListLabelElementUI*)GetItemAt(0);
	pNode = (CFListNode*)pListElement->GetTag();
	AddChildFolder(pNode);
	SetChildVisible(pNode,true);

	const int BUFSIZE = 256;
	TCHAR szLogicDriveStrings[BUFSIZE];
	ZeroMemory(szLogicDriveStrings, BUFSIZE);
	GetLogicalDriveStrings(BUFSIZE-1, szLogicDriveStrings);
	TCHAR *szDrive = szLogicDriveStrings;
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentlength;
	DWORD dwFileSystemFlags;
	TCHAR szFileSystemNameBuffer[BUFSIZE];
	TCHAR szDriveName[MAX_PATH];
	wstring fileSystem = _T("NTFS");
	t_string strName;
	do 
	{	
		if (!(GetVolumeInformation(szDrive, szDriveName, MAX_PATH, &dwVolumeSerialNumber,
			&dwMaximumComponentlength, &dwFileSystemFlags, szFileSystemNameBuffer, BUFSIZE)))
		{
			szDrive += (lstrlen(szDrive)+1);
			continue;
		}

		UINT driveType = GetDriveType(szDrive);

		if (driveType == DRIVE_UNKNOWN || driveType == DRIVE_REMOTE)
		{
			szDrive += (lstrlen(szDrive)+1);
			continue;
		}

		wstring strDriveTmp = szDrive;
		if (fileSystem.compare(szFileSystemNameBuffer) == 0)
		{
			strName = _T("本地磁盘");
		}
		else
		{
			strName = szDriveName;
		}
		strName += _T("(");
		strName += strDriveTmp.substr(0,2);
		strName += _T(")");

		pNode = AddFolder(strName.c_str(), szDrive, FOLDER_TYPE_DISK, NULL);
		SetChildVisible(pNode,false);
		szDrive += (lstrlen(szDrive)+1);
	} while (*szDrive !='\x00');
	//ShowFolder(wstring(_T("E:\\UI修改细节1\\UI修改细节\\切图\\images\\images")));
}
CFolderListUI::~CFolderListUI(void)
{
	if (root)
		delete root;
}
void CFolderListUI::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("itemclick"))
	{
		if (_tcsicmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0) 
		{
			CFListNode *pNode = (CFListNode*)msg.pSender->GetTag();
			if (!pNode || (pNode->nLevel == 0 && /*pNode->strText == _T("我的电脑")*/_tcscmp(pNode->strText.c_str(),  _T("我的电脑")) == 0))
				return;
			CollBrotherFolders(pNode);
			AddChildFolder(pNode);
			SetChildVisible(pNode, !pNode->bChildVisble);
			int index = GetItemIndex(msg.pSender);
			if(index!=m_lastSelectIndex)
			{
				m_lastSelectIndex = index;
				m_pManager->SendNotify(m_pParent,_T("directorychange"));
			}
		}
	}
	else if (msg.sType == _T("itemactivate")) 
	{
		if( _tcsicmp(msg.pSender->GetClass(), _T("ListLabelElementUI")) == 0 ) 
		{
			CFListNode* pNode = (CFListNode*)msg.pSender->GetTag();
			if(pNode)
			{
				CollBrotherFolders(pNode);
				AddChildFolder(pNode);
				SetChildVisible(pNode, !pNode->bChildVisble);
			}
		}
	}
}
void CFolderListUI::DoEvent(TEventUI& event) 
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) 
	{
		if (m_pParent != NULL) 
			m_pParent->DoEvent(event);
		else 
			CVerticalLayoutUI::DoEvent(event);
		return;
	}

	if (event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID)
	{
		if (m_dwDelayLeft > 0) 
		{
			--m_dwDelayLeft;
			SIZE sz = GetScrollPos();
			LONG lDeltaY =  (LONG)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
			if ((lDeltaY > 0 && sz.cy != 0) || (lDeltaY < 0 && sz.cy != GetScrollRange().cy )) 
			{
				sz.cy -= lDeltaY;
				SetScrollPos(sz);
				return;
			}
		}
		m_dwDelayDeltaY = 0;
		m_dwDelayNum = 0;
		m_dwDelayLeft = 0;
		m_pManager->KillTimer(this, SCROLL_TIMERID);
		return;
	}

	if (event.Type == UIEVENT_SCROLLWHEEL)
	{
		LONG lDeltaY = 0;
		if (m_dwDelayNum > 0) 
			lDeltaY =  (LONG)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
		switch (LOWORD(event.wParam)) 
		{
		case SB_LINEUP:
			if (m_dwDelayDeltaY >= 0) 
				m_dwDelayDeltaY = lDeltaY + 8;
			else 
				m_dwDelayDeltaY = lDeltaY + 12;
			break;
		case SB_LINEDOWN:
			if (m_dwDelayDeltaY <= 0) 
				m_dwDelayDeltaY = lDeltaY - 8;
			else 
				m_dwDelayDeltaY = lDeltaY - 12;
			break;
		}
		if (m_dwDelayDeltaY > 100) 
			m_dwDelayDeltaY = 100;
		else if (m_dwDelayDeltaY < -100) 
			m_dwDelayDeltaY = -100;
		m_dwDelayNum = (DWORD)sqrt((double)abs(m_dwDelayDeltaY)) * 5;
		m_dwDelayLeft = m_dwDelayNum;
		m_pManager->SetTimer(this, SCROLL_TIMERID, 50U);
		return;
	}

	CListUI::DoEvent(event);
}

void CFolderListUI::RemoveAll()
{
	CListUI::RemoveAll();

	delete root;
	root = new CFListNode;
	root->nLevel = -1;
	root->bChildVisble = true;
	root->pListElement = NULL;
}

CFListNode* CFolderListUI::AddNode(LPCTSTR pszText, LPCTSTR pszPath, int nType, CFListNode* pParent)
{
	if (!pParent)
		pParent = root;
	CListLabelElementUI* pListElement = new CListLabelElementUI;
	CFListNode* pNode = new CFListNode;
	pNode->nLevel = pParent->nLevel + 1;
	if (pNode->nLevel == 0)
		pNode->bChildVisble = true;
	else
		pNode->bChildVisble = false;
	if (pszText)
		pNode->strText = pszText;
	if (pszPath)
		pNode->strPath = pszPath;
	pNode->pListElement = pListElement;
	pNode->nType = nType;

	if (!pParent->bChildVisble)
		pListElement->SetVisible(false);
	if (pParent != root && !pParent->pListElement->IsVisible())
		pListElement->SetVisible(false);

	t_string htmlText = _T("<x 5>");
	for (int i=0; i<pNode->nLevel; i++)
		htmlText += _T("<x 24>");
	if(HasChildFolder(pszPath))
	{
	  if (pNode->bChildVisble)
		  htmlText += _T("<a><i tree_expand.png 2 1></a>");
	  else
		  htmlText += _T("<a><i tree_expand.png 2 0></a>");
	  //htmlText += _T("<x 15>");
	}
	else
	{
		htmlText += _T("<x 15>");
		htmlText += _T("<y 0>");
	}
	TCHAR chTemp[64] = {0};
	_stprintf(chTemp, _T("<a><i tree_type_%d.png></a> "), pNode->nType);
	htmlText += chTemp;
	htmlText += pNode->strText;

	pListElement->SetText(htmlText.c_str());
	pListElement->SetTag((UINT_PTR)pNode);
//	if (pNode->nLevel == 0)
//		pListElement->SetBkImage(_T("file='tree_top.png' corner='2,1,2,1' fade='100'"));

	int nIdx = 0;
	if (pParent->children.size())
	{
		CFListNode* pPrev = pParent->GetLastChild();
		nIdx = pPrev->pListElement->GetIndex() + 1;
	}
	else
	{
		if (pParent == root)
			nIdx = 0;
		else
			nIdx = pParent->pListElement->GetIndex() + 1;
	}

	if (!CListUI::AddAt(pListElement, nIdx))
	{
		delete pListElement;
		delete pNode;
		pNode = NULL;
	}
	pListElement->SetFixedHeight(30);
	
	pParent->AddChild(pNode);

	return pNode;
}

void CFolderListUI::RemoveNode(CFListNode* pNode)
{
	if (!pNode || pNode == root)
		return;
	int a = pNode->children.size();
	while (pNode->children.size()>0)
	{
		PMFolderNodeList::iterator iter = m_folderList.find(pNode->children[0]->strPath);
		if (iter != m_folderList.end())
		{
			m_folderList.erase(iter);
		}
		RemoveNode(pNode->children[0]);
	}
	PMFolderNodeList::iterator iter = m_folderList.find(pNode->strPath);
	if (iter != m_folderList.end())
	{
		m_folderList.erase(iter);
	}
	CListUI::Remove(pNode->pListElement);
	//delete pNode->pListElement;	
	pNode->pParent->RemoveChild(pNode);
	//delete pNode;
}
void CFolderListUI::RemoveChildFolder(CFListNode* pNode)
{
  if (!pNode || pNode == root)
	return;
  for (int i=0; i<pNode->children.size(); i++)
  {
	CListUI::Remove(pNode->children[i]->pListElement);
  }
  pNode->children.clear();
  FindFolderFiles(pNode);
  CreateSearchFolderThread(pNode);
  if(pNode->bChildVisble)
	SetChildVisible(pNode,true);
}
void CFolderListUI::Refush()
{
  CFListNode pNodeTemp,*pNodeOld,*pNodeNew = NULL;
  CFListNode *p = new CFListNode;
  CListLabelElementUI* pListElement = (CListLabelElementUI*)GetItemAt(m_lastSelectIndex);
  if(pListElement)
  {
	pNodeOld = (CFListNode *)pListElement->GetTag();
	pNodeTemp = *pNodeOld;

	RemoveNode(pNodeOld);
	t_string folerPath = pNodeTemp.strPath;
	DWORD ret = GetFileAttributes((LPCTSTR)folerPath.c_str()); 
	if (ret != INVALID_FILE_ATTRIBUTES)
	{
	  pNodeNew = AddFolder(pNodeTemp.strText.c_str(),pNodeTemp.strPath.c_str(),pNodeTemp.nType,pNodeTemp.pParent);
	  vector<CFListNode*>::iterator iFind = find(pNodeNew->pParent->children.begin(),pNodeNew->pParent->children.end(),(CFListNode *)GetItemAt(m_lastSelectIndex)->GetTag());
	  p = pNodeNew;
	  p->pParent->children.insert(iFind,p);
	  p->pParent->children.pop_back();
	  p->pListElement->SetTag((UINT_PTR)p);
	  SetItemIndex(p->pListElement,m_lastSelectIndex);
	  SelectItem(m_lastSelectIndex); 
	  FindFolderFiles(p);
	  CreateSearchFolderThread(p);
	  SetChildVisible(p,pNodeTemp.bChildVisble);
	  p->bChildVisble = pNodeTemp.bChildVisble;
	  p->bChildAdded = true;
	}
	else
	{
	  m_lastSelectIndex = GetCurSel();
	}
  }
}
void CFolderListUI::SetChildVisible(CFListNode* pNode, bool bVisible)
{
	if (!pNode || pNode == root) 
		return;

	if (pNode->bChildVisble == bVisible)
		return;
	pNode->bChildVisble = bVisible;

	t_string htmlText = _T("<x 6>");
	for (int i=0; i<pNode->nLevel; ++i)
	{
		htmlText += _T("<x 24>");
	}
	if (HasChildFolder(pNode->strPath.c_str())) 
	{
		if (pNode->bChildVisble) 
			htmlText += _T("<a><i tree_expand.png 2 1></a>");
		else 
			htmlText += _T("<a><i tree_expand.png 2 0></a>");
	}
	else
	{
	  htmlText += _T("<x 15>");
	}
	TCHAR chTemp[64] = {0};
	_stprintf(chTemp, _T("<a><i tree_type_%d.png></a> "), pNode->nType);
	htmlText += chTemp;
	htmlText += pNode->strText;
	pNode->pListElement->SetText(htmlText.c_str());

	if (!pNode->pListElement->IsVisible()) 
		return;
	if (!pNode->children.size()) 
		return;

	CFListNode* pBegin = pNode->children[0];
	CFListNode* pEnd = pNode->GetLastChild();
	for (int i=pBegin->pListElement->GetIndex(); i<=pEnd->pListElement->GetIndex(); ++i)
	{
		CControlUI* control = GetItemAt(i);
		if (_tcsicmp(control->GetClass(), _T("ListLabelElementUI")) == 0)
		{
			if (!bVisible) 
			{
				control->SetVisible(false);
			}
			else
			{
				CFListNode* pParent = ((CFListNode*)control->GetTag())->pParent;
				if (pParent->bChildVisble && pParent->pListElement->IsVisible())
				{
					control->SetVisible(true);
				}
			}
		}
	}
}

SIZE CFolderListUI::GetExpanderSizeX(CFListNode* pNode) const
{
	if (!pNode || pNode == root) 
		return CSize();
	if (pNode->nLevel >= 3) 
		return CSize();

	SIZE szExpander = {0};
	szExpander.cx = 6 + 24 * pNode->nLevel - 4;		//适当放大一点
	szExpander.cy = szExpander.cx + 16 + 8;			//适当放大一点
	return szExpander;
}

CFListNode* CFolderListUI::AddFolder(LPCTSTR pszText, int nCsidl, int nType, CFListNode* pParent)
{
	TCHAR szPath[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, szPath, nCsidl, FALSE))
	{
		return AddFolder(pszText, szPath, nType, pParent);
	}
	return NULL;
}

CFListNode* CFolderListUI::AddFolder(LPCTSTR pszText, LPCTSTR pszPath, int nType, CFListNode* pParent)
{
	CFListNode* pNode = AddNode(pszText, pszPath, nType, pParent);

	PMFolderNodeList::iterator iter = m_folderList.find(pNode->strPath);
	if (iter != m_folderList.end())
	{
		m_folderList.erase(iter);
	}

	m_folderList.insert(make_pair<wstring, CFListNode*>(pNode->strPath, pNode));

	return pNode;
}
BOOL CFolderListUI::CheckValidPicFMT(WIN32_FIND_DATA& fd)
{
	if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;
// 	if (CPMImageLoadMgr::IsValidImageByName(fd.cFileName))
// 	{
// 		return TRUE;
// 	}
	return FALSE;
}
void CFolderListUI::FindFolderFiles(CFListNode* pNode)
{
	if (pNode->files.size())
		pNode->files.clear();
	t_string strPath = pNode->strPath;
	strPath += _T("\\*.*");
	WIN32_FIND_DATA fd;
	HANDLE hFile = FindFirstFile((LPCTSTR)strPath.c_str(), &fd); 
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::FindClose(hFile);
		return;
	}
	do
	{
		if ((_tcscmp(fd.cFileName, _T(".")) == 0 || _tcscmp(fd.cFileName, _T("..")) == 0))
			continue;
		if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
		   if (CheckValidPicFMT(fd))
		   {
			 pNode->files.push_back(fd);
		   }
		}
		else
		{
			continue;
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{
				continue;
			}
			strPath = _tcscat((wchar_t *)pNode->strPath.c_str(), _T("\\"));
			strPath += fd.cFileName;
			strPath += _T("\\");
			AddFolder(fd.cFileName, strPath.data(), FOLDER_TYPE_FOLDER, pNode);
		}
	}
	while (FindNextFile(hFile, &fd));
	::FindClose(hFile);
}
void CFolderListUI::AddChildFolder(CFListNode* pNode)
{
	if (pNode->bChildAdded)
	   return;
	pNode->bChildAdded = true;
	FindFolderFiles(pNode);
	CreateSearchFolderThread(pNode);
}
void CFolderListUI::CollBrotherFolders(CFListNode *pNode)
{
  CFListNode *pNodeParent = pNode->pParent;
  if(pNodeParent)
  {
	for (int i=0; i<pNodeParent->children.size(); i++)
	{
	  if((pNodeParent->children[i]->bChildVisble)&&(pNodeParent->children[i]!=pNode))
	    SetChildVisible(pNodeParent->children[i],false);
	}
  }
}
BOOL CFolderListUI::HasChildFolder(LPCTSTR strPath)
{
  t_string szPath = strPath;
  szPath += _T("\\*.*");
  WIN32_FIND_DATA fd;
  HANDLE hFile = FindFirstFile((LPCTSTR)szPath.c_str(), &fd); 
  if (hFile == INVALID_HANDLE_VALUE)
  {
	::FindClose(hFile);
	return FALSE;
  }
  do
  {
	if ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || _tcscmp(fd.cFileName, _T(".")) == 0 
	  || _tcscmp(fd.cFileName, _T("..")) == 0)
	  continue;
	if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
	  ::FindClose(hFile);
	  return TRUE;
	}
  }
  while (FindNextFile(hFile, &fd));
  ::FindClose(hFile);
  return FALSE;
}
CFListNode* CFolderListUI::GetCurrentNode()
{
	CFListNode *pNode = NULL;
	CListLabelElementUI* pListElement = (CListLabelElementUI*)GetItemAt(m_lastSelectIndex);
	if(pListElement)
	{
		 pNode = (CFListNode *)pListElement->GetTag();
	}
	return pNode;
}
wstring CFolderListUI::GetCurrentPath()
{
	wstring strPath;
	CListLabelElementUI* pListElement = (CListLabelElementUI*)GetItemAt(m_lastSelectIndex);
	if(pListElement)
	{
		CFListNode *pNode = (CFListNode *)pListElement->GetTag();
		strPath = pNode->strPath;
	}
    return strPath;
}
void CFolderListUI::ShowFolder(wstring &strPath)
{
	int pos = 0;
	int found = 0;
	CFListNode *pNode = root;
	if(strPath.empty()||(root == NULL))
		return;
	if(root->children[0])
	   SetChildVisible(root->children[0], false);
	if(strPath.find_last_of(_T("\\"))<strPath.length()-1)
		strPath += _T("\\");
	while (found != wstring::npos)
	{
		found = strPath.find(_T("\\"), pos);
		wstring str = wstring(strPath, 0, found+1);
		if(str.empty())
		{
			str = strPath;
		}
		for(int i = pNode->children[0]->pListElement->GetIndex();i<=pNode->GetLastChild()->pListElement->GetIndex();i++)
		{
			CListLabelElementUI *element = (CListLabelElementUI *)GetItemAt(i);
			CFListNode *pNodeTemp = (CFListNode *)element->GetTag();
			if(/*pNodeTemp->strPath == str*/0 == _tcscmp(pNodeTemp->strPath.c_str(), str.c_str()))
			{
				AddChildFolder(pNodeTemp);
				SetChildVisible(pNodeTemp, true);
				pNode = pNodeTemp;
				break;
			}
		}
		if(/*str == strPath*/0 == _tcscmp(str.c_str(), strPath.c_str()))
		{
			pNode->pListElement->Select(true);
			m_lastSelectIndex = pNode->pListElement->GetIndex();
			break;
		}
		pos = found + 2;
	}
}

void CFolderListUI::OnFindNewFolderItem(SearchFolderThreadMsgInfo* pInfo, DWORD iNums)
{
	if (pInfo == NULL)
	{
		return;
	}

	if (pInfo->strParentPath.empty())
	{
		delete pInfo;
		pInfo = NULL;

		return;
	}

	PMFolderNodeList::iterator iter = m_folderList.find(pInfo->strParentPath);
	if (iter != m_folderList.end())
	{
		CFListNode *pNode = iter->second;
		if (pNode != NULL && pNode == pInfo->pParent)
		{
			if (pInfo->type == FOLDER_CHILD_TYPE_FILE)
			{
				if (iNums == 1)
				{
					if (pNode->files.size())
					{
						pNode->files.clear();
					}
				}

				pNode->files.push_back(pInfo->fd);
			}
			else
			{
				AddFolder(pInfo->pszText.c_str(), pInfo->strPath.c_str(), FOLDER_TYPE_FOLDER, pNode);
			}
		}
	}

	delete pInfo;
	pInfo = NULL;
}

void CFolderListUI::SearchFolderFiles(wstring &strParentPath, CFListNode *pNode)
{
	wstring strTmp = strParentPath;
	strTmp += _T("\\*.*");
	wstring strPath;
	WIN32_FIND_DATA fd;

	HANDLE hFile = FindFirstFile((LPCTSTR)strTmp.c_str(), &fd); 
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::FindClose(hFile);
		return;
	}
	DWORD iNums = 0;
	DWORD iFolders = 0;
	do
	{
		if ((_tcscmp(fd.cFileName, _T(".")) == 0 || _tcscmp(fd.cFileName, _T("..")) == 0))
			continue;

		SearchFolderThreadMsgInfo* pInfo = new SearchFolderThreadMsgInfo;
		pInfo->pParent = pNode;
		bool bVal = false;
		if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			continue;
			if (CheckValidPicFMT(fd))
			{
				pInfo->type = FOLDER_CHILD_TYPE_FILE;
				pInfo->strParentPath = strParentPath;
				memcpy(&(pInfo->fd), &fd, sizeof(WIN32_FIND_DATA));
				++iNums;
				bVal = true;
			}
		}
		else
		{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{
				delete pInfo;
				pInfo = NULL;
				continue;
			}
			strPath = _tcscat((wchar_t *)strParentPath.c_str(), _T("\\"));//strParentPath + _T("\\");
			strPath += fd.cFileName;
			strPath += _T("\\");
			pInfo->type = FOLDER_CHILD_TYPE_FOLDER;
			pInfo->strParentPath = strParentPath;
			pInfo->strPath = strPath;
			pInfo->pszText = fd.cFileName;
			bVal = true;
			++iFolders;
		}

		if (m_pManager != NULL && bVal)
		{
//			::PostMessage(m_pManager->GetPaintWindow(), WM_FIND_NEW_FOLDER_ITEM, WPARAM(pInfo), iNums);
			//wchar_t buffer[32] = {0};
			//_itow(iFolders, buffer, 10);
			//OutputDebugString(buffer);
			if (iFolders > 400)
			{
				Sleep(300);
			}
			else if (iFolders > 100)
			{
				Sleep(40);
			}
			else
			{
				Sleep(1);
			}
		}
		else
		{
			delete pInfo;
			pInfo = NULL;
		}
	}
	while (FindNextFile(hFile, &fd));
	::FindClose(hFile);
}

void CFolderListUI::CreateSearchFolderThread(CFListNode *pNode)
{
	SearchFolderThreadMsgInfo* pInfo = new SearchFolderThreadMsgInfo;
	if (pInfo == NULL)
	{
		return;
	}

	pInfo->strParentPath = pNode->strPath;
	pInfo->pFolderListUI = this;
	pInfo->pParent = pNode;

	unsigned int threadid = 0;
	HANDLE threadTask = (HANDLE)_beginthreadex(NULL, 0, &CFolderListUI::SearchFolderThreadFunc, (void*)pInfo, CREATE_SUSPENDED, &threadid);

	if (threadTask == NULL)
	{
		threadTask = INVALID_HANDLE_VALUE ;

		delete pInfo;
		pInfo = NULL;
	}
	else
	{
		if (::ResumeThread(threadTask) == (DWORD)-1)
		{
			::CloseHandle(threadTask) ;
			threadTask = INVALID_HANDLE_VALUE ;
			delete pInfo;
			pInfo = NULL;
		}
	}
}


unsigned int WINAPI CFolderListUI::SearchFolderThreadFunc(void* pArg)
{
	SearchFolderThreadMsgInfo* pInfo = (SearchFolderThreadMsgInfo*)pArg;

	if (pInfo == NULL)
	{
		return 0;
	}

	if (pInfo->pFolderListUI == NULL || pInfo->strParentPath.empty())
	{
		delete pInfo;
		pInfo = NULL;

		return 0;
	}

	pInfo->pFolderListUI->SearchFolderFiles(pInfo->strParentPath, pInfo->pParent);

	delete pInfo;
	pInfo = NULL;

	return 0;
}