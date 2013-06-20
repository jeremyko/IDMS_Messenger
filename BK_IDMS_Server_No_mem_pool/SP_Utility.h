// IDMSCSP_Utility.h: interface for the IDMSCSP_Utility class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFXCSP_Utility_H__35C4618C_A1AE_40C8_901C_AD32154F1762__INCLUDED_)
#define AFXCSP_Utility_H__35C4618C_A1AE_40C8_901C_AD32154F1762__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#  pragma warning ( disable : 4355 4284  4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning ( disable : 4245 4503 4514 4660 4715) 

// 현재 이 소스는 Local 범위를 벗어나면 할당(new) 메모리를 자동으로 삭제(delete)하는데 사용됨.

//////////////////////////////////////////////////////////////////
// smart pointer version: 1.0
// by bro ( bro@shinbiro.com ) 2003-07-07
// std::auto_ptr is not safe for return value of function.
template <class _T>
class CSP_Utility
{
private:
	class obj
	{
	public:
		obj(_T* _p){ ref = 0; p = _p; addref(); }
		~obj(){ release(); }
		void addref(){ ref++; }
		_T* get(){ return p; }
		void release()
		{ 
			if( --ref == 0 ) 
			{ 
				if( p ) 
				{ 
					delete p; 
					p=0; 
				} 
				delete this; 
			} 
		}
		_T* p;
		int ref;
	};

	obj* _o; 

public:
	CSP_Utility() { _o = 0; }
	CSP_Utility( bool bNew ) { bNew ? _o = new obj(new _T) : _o = 0; }  
	CSP_Utility( int bNew ) { bNew ? _o = new obj(new _T) : _o = 0; }  
	CSP_Utility(_T* p) { _o = new obj(p); }
	~CSP_Utility() { if( _o ) _o->release(); }

	CSP_Utility( const CSP_Utility<_T>& sp2 ) { _o = sp2._o; if(_o)_o->addref(); }
	CSP_Utility& operator = ( const CSP_Utility& sp2 )
	{
		if( this == &sp2 ) return *this;

		if( _o ) _o->release(); 
		_o = sp2._o; if(_o)_o->addref();
		return *this;
	}

	CSP_Utility& operator = ( _T* p )
	{
		if( _o ) if( _o->p == p ) return *this;

		if( _o ) _o->release(); 
		if( !p ){ _o = 0; return *this; }
		_o = new obj(p);
		return *this;
	}

	_T& operator*() const { return *(_o->get()); }	
	_T *operator->() const {return (_o->p); }
	_T *get() const { if(!_o) return NULL; return _o->p; }

	operator bool()
		{ if( _o && _o->p ) return true; return false; }

	friend bool operator==(const CSP_Utility<_T>& sp, const _T* p);
	friend bool operator==(const CSP_Utility<_T>& sp, int n);
	bool operator !() { return !operator bool(); }
};


template<class _T> inline
bool operator==(const CSP_Utility<_T>& sp1, const CSP_Utility<_T>& sp2)
{ if( sp1._o && sp2._o ) return (sp1._o->p == sp2._o->p); return false; }

template<class _T> inline
bool operator==(const CSP_Utility<_T>& sp, const _T* p)
{ if( sp->_o ) return ( sp->_o->p == p ); return false;}

template<class _T> inline
bool operator==(const CSP_Utility<_T>& sp, int n)
{	
	int tmp = 0;
	if( !sp._o ) tmp = 0;
	else
	{
		if( !sp._o->p ) tmp = 0; 
		else tmp = (int)(sp._o->p); 
	}
	return (tmp == n );
}


#endif // !defined(AFXCSP_Utility_H__35C4618C_A1AE_40C8_901C_AD32154F1762__INCLUDED_)
