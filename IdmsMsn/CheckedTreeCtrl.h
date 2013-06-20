#ifndef AFX_CHECKEDTREECTRL_H__9AAC989F_A17B_11D3_853E_006097B7EE40__INCLUDED_
#define AFX_CHECKEDTREECTRL_H__9AAC989F_A17B_11D3_853E_006097B7EE40__INCLUDED_

// CheckedTreeCtrl.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Fenster CCheckedTreeCtrl 

#include <afxext.h>

class CCheckedTreeCtrl : public CTreeCtrl
{
// Konstruktion
public:
	CCheckedTreeCtrl();

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CCheckedTreeCtrl)
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CCheckedTreeCtrl();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CCheckedTreeCtrl)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	// Method to set the images for the tree-control
	void
	SetBitmaps(
		UINT NormalId,	// Resource-Id of the item-images
		UINT StatusId	// Resource-Id of the checkbox-images
	);

	// Method to switch the state of the checkbox in the selected item
	void changeItemState();	

private:
	//imagelist for item-images
	CImageList	m_NormalImageList;

	//imagelist for checkbox-images fuer Statusbilder der Elemente
	// !!! KEEP CARE !!!
	// !!! LET IMAGE WITH INDEX 0 EMPTY AND DO NOT USE IT !!!
	CImageList	m_StatusImageList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio fügt zusätzliche Deklarationen unmittelbar vor der vorhergehenden Zeile ein.

#endif // AFX_CHECKEDTREECTRL_H__9AAC989F_A17B_11D3_853E_006097B7EE40__INCLUDED_
