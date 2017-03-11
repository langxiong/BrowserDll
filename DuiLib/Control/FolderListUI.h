#pragma once

//#include "stdafx.h"
#include <vector>
#include <map>
#include <string>

using namespace std;


//class CListLabelElementUI;

#include "UIList.h"

using namespace DuiLib;

//#include "/1188press/source/1188compress/trunk/src/duilib/DuiLib/Control/UIList.h"

#ifdef _UNICODE
typedef std::wstring t_string;
#else
#define std::string t_string;
#endif

#define FOLDER_TYPE_DESKTOP 1
#define FOLDER_TYPE_MYDOCS	2
#define FOLDER_TYPE_MYPICS	3
#define FOLDER_TYPE_DISK	4
#define FOLDER_TYPE_FOLDER	5
struct CFListNode
{
	typedef vector<CFListNode*> VCChildren;
	typedef vector<WIN32_FIND_DATA> LSTFile;
	VCChildren children;
	LSTFile files;
	CFListNode* pParent;

	int nType;				// 文件夹类型（桌面、我的文档、图片收藏...）
	int nLevel;
	bool bChildVisble;
	bool bChildAdded;
	t_string strText;
	t_string strPath;
	CListLabelElementUI* pListElement;
public:
	CFListNode() : pParent(NULL), bChildAdded(false), nType(FOLDER_TYPE_FOLDER) {}
	~CFListNode()
	{
		for (int i=0; i<children.size(); i++)
			delete children[i];
		children.clear();
	}

	void AddChild(CFListNode* pChild)
	{
		pChild->pParent = this;
		children.push_back(pChild);
	}
	void RemoveChild(CFListNode* pChild)
	{
		VCChildren::iterator iter = children.begin();
		for (; iter != children.end(); iter++)
		{
			if (*iter == pChild)
			{
				children.erase(iter);
				return;
			}
		}
	}

	CFListNode* GetLastChild()
	{
		size_t sz = children.size();
		if (sz)
			return (children[sz - 1])->GetLastChild();
		else
			return this;
	}	
};

typedef map<t_string, CFListNode*> PMFolderNodeList;

enum FolderChildType
{
	FOLDER_CHILD_TYPE_FILE = 1,    // 文件
	FOLDER_CHILD_TYPE_FOLDER,      // 目录
};
class CFolderListUI;

struct SearchFolderThreadMsgInfo
{
	FolderChildType type;
	wstring strPath;
	wstring pszText;
	wstring strParentPath;
	WIN32_FIND_DATA fd;
	CFolderListUI* pFolderListUI;
	CFListNode* pParent;

	SearchFolderThreadMsgInfo()
		: type(FOLDER_CHILD_TYPE_FILE),
		  pFolderListUI(NULL),
		  pParent(NULL)
	{
		ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));
	}
};

class CFolderListUI : public CListUI , public INotifyUI
{
	enum { SCROLL_TIMERID = 10 };
public:
	CFolderListUI();
	~CFolderListUI(void);
	void RemoveAll();
	CFListNode* AddNode(LPCTSTR pszText, LPCTSTR pszPath, int nType, CFListNode* pParent = NULL);
	void RemoveNode(CFListNode* pNode);
	void SetChildVisible(CFListNode* pNode, bool bVisible);
	SIZE GetExpanderSizeX(CFListNode* pNode) const;

	CFListNode* AddFolder(LPCTSTR pszText, LPCTSTR pszPath, int nType, CFListNode* pParent);
	CFListNode* AddFolder(LPCTSTR pszText, int nCsidl, int nType, CFListNode* pParent);
	void AddChildFolder(CFListNode* pNode);
	void RemoveChildFolder(CFListNode* pNode);
	BOOL CheckValidPicFMT(WIN32_FIND_DATA& fd);
	void Refush();
	void CollBrotherFolders(CFListNode *pNode);
	BOOL HasChildFolder(LPCTSTR strPath);
	CFListNode* GetCurrentNode();
	wstring GetCurrentPath();
	void ShowFolder(wstring &strPath);

public:
	void CreateSearchFolderThread(CFListNode *pNode);
	static unsigned int WINAPI SearchFolderThreadFunc(void* pArg);
	void SearchFolderFiles(wstring &strPath, CFListNode *pNode);
	void OnFindNewFolderItem(SearchFolderThreadMsgInfo* pInfo, DWORD iNums);

protected:
	void Init();
	void DoEvent(TEventUI& event);
	void Notify(TNotifyUI& msg);
	double CalculateDelay(double state) 
	{
		return pow(state, 2);
	}
	void FindFolderFiles(CFListNode* pNode);
private:
	CFListNode* root;
	LONG m_dwDelayDeltaY;
	DWORD m_dwDelayNum;
	DWORD m_dwDelayLeft;
	INT m_lastSelectIndex;
//	CPMImageLoadMgr* m_imgLoadMgr;
	PMFolderNodeList m_folderList;
};

