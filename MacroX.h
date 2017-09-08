#if !defined(__MACRO_X_BY_JADE_H_)
#define __MACRO_X_BY_JADE_H_

#include <stdarg.h>

#define NOTHROW_NEW(I,O) do{try{ O = new I; }catch(std::bad_alloc){O=0;}}while(0)

#define NOTHROW_NEW_RET(I,O,R) do{try{O=new I;if(O==0){ R; return 0;} }	  \
							   catch(std::bad_alloc){O=0; R; return 0;}}while(0)

//use convert BOOL to bool
#define BOOL2bool(X) (X?true:false)

//use define interface
#define CLASS_I(r,f) virtual r f = 0;

// MSVC_SUPPRESS_WARNING disables warning |n| for the remainder of the line and
// for the next line of the source file.
#define MSVC_SUPPRESS_WARNING(n) __pragma(warning(suppress:n))

// MSVC_PUSH_DISABLE_WARNING pushes |n| onto a stack of warnings to be disabled.
// The warning remains disabled until popped by MSVC_POP_WARNING.
#define MSVC_PUSH_DISABLE_WARNING(n) __pragma(warning(push)) \
	__pragma(warning(disable:n))

// MSVC_PUSH_WARNING_LEVEL pushes |n| as the global warning level.  The level
// remains in effect until popped by MSVC_POP_WARNING().  Use 0 to disable all
// warnings.
#define MSVC_PUSH_WARNING_LEVEL(n) __pragma(warning(push, n))

// Pop effects of innermost MSVC_PUSH_* macro.
#define MSVC_POP_WARNING() __pragma(warning(pop))

#define MSVC_DISABLE_OPTIMIZE() __pragma(optimize("", off))
#define MSVC_ENABLE_OPTIMIZE() __pragma(optimize("", on))

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)		\
		TypeName(const TypeName&);              \
		void operator=(const TypeName&)

//获取list成员的宏,有范围检查
#define SAFE_LIST_GET(tmplist,iIndex) if (iIndex < 0 && iIndex > tmplist.size())	{return NULL;}	return tmplist.at(iIndex);

//清空列表
#define LIST_EMPTY(tmplist) while (!tmplist.empty()) {delete tmplist.back();	tmplist.pop_back();} 

#define SAFE_DELETE(item)  if(NULL!=item) {delete item;item=NULL;}

#define GET_JSON_STRING_VALUE(root,nodename)  CString strTmp;strTmp.Empty(); if(root[nodename].isNull()) {return strTmp;} else {return CA2T(root[nodename].asString().c_str());}

/* template tools */
// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

//数据成员获取和赋值宏声明
#define CONST_IDECLARE_FUN(x,t,I)	public:  const t& get##x##() const;		\
										 void  put##x##(const t& );			\
									private: I m_##x;
//数据成员获取和赋值宏实现
#define CONST_IMPLETE_FUN(x,t,c)	const t& c::get##x##() const{ return m_##x; }	\
									void c::put##x##(const t& data){ m_##x = data; }

//数据成员获取和赋值宏声明
#define	IDECLARE_FUN(x,t,I)		public:  t get##x##() const;				\
										 void  put##x##(t );				\
								private: I m_##x;
//数据成员获取和赋值宏实现
#define IMPLETE_FUN(x,t,c)		t	 c::get##x##() const{ return m_##x; }	\
								void c::put##x##(t data){ m_##x = data; }

//数据成员接口声明
#define	IDECLARE_VR(x,t)		public: virtual t get##x##() const = 0;		\
										virtual void  put##x##(t ) = 0;		\
										// check the pointer is null and return null

#define PTR_NULL(p)				{ assert(p); if (NULL == p) return NULL; }
#define PTR_VOID(p)				{ assert(p); if (NULL == p) return; }
#define PTR_FALSE(p)			{ assert(p); if (NULL == p) return false; }

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if (NULL != p) { delete (p); (p) = NULL; } }
#endif

#ifndef COUNT_OF
#define COUNT_OF(array)			(sizeof(array)/sizeof(array[0]))
#endif

//类声明转字符串 CLASS_NAME
#define CLASS_NAME_A(X) (#X)
#define CLASS_NAME_W(X) (L#X)

#if defined(_UNICODE)
#define CLASS_NAME CLASS_NAME_W
#else
#define CLASS_NAME CLASS_NAME_A
#endif

//duilib类型接口声明+定义 DUI_DECLARE
#define DUI_CLASS_A(X) LPCTSTR GetClass() const{ return #X;		}
#define DUI_CLASS_W(X) LPCTSTR GetClass() const{ return L#X;	}

#if defined(_UNICODE)
#define DUI_CLASS DUI_CLASS_W
#else
#define DUI_CLASS DUI_CLASS_A
#endif
//for duilib interface declare
#define DUI_DECLARE(X,B)	DUI_CLASS(X)			\
	LPVOID GetInterface(LPCTSTR pstrName){			\
	if( _tcscmp(pstrName,GetClass())==0)			\
		return static_cast<X*>(this);				\
	return B::GetInterface(pstrName);	 }
//用于批量生成枚举值和返回值的映射
#define BEGIN_ENUM(enumX,enumT)	static LPCTSTR enumX##_TIP(enumT t) {	
#define DECLARE_ENUM(enumV,R)	if(enumV==t) return R;


#define BEGIN_ENUM_2(enumX,enum1,enum2)	static LPCTSTR enumX##_TIP(enum1 t1,enum2 t2) {	
#define DECLARE_ENUM_2(enum1,enum2,R)	if(enum1==t1 && enum2==t2) return R;

#define END_ENUM() return _T(""); }


// sprintf 用的一些格式化字符
#define CHAR_FORMAT                             "%c"
#define POINTER_FORMAT                          "%p"
#define STRING_FORMAT                           "%s"

#define INT8_FORMAT                             "%d"
#define UINT8_FORMAT                            "%u"
#define INT16_FORMAT                            "%d"
#define UINT16_FORMAT                           "%u"
#define INT32_FORMAT                            "%d"
#define UINT32_FORMAT                           "%u"

#define HEX8_FORMAT                             "%x"
#define HEX16_FORMAT                            "%x"
#define HEX32_FORMAT                            "%x"

#define OCT8_FORMAT                             "%o"
#define OCT16_FORMAT                            "%o"
#define OCT32_FORMAT                            "%o"

#define FLOAT_FORMAT                            "%f"
#define DOUBLE_FORMAT                           "%lf"

#define INT64_FORMAT                            "%I64d"
#define UINT64_FORMAT                           "%I64u"
#define HEX64_FORMAT                            "%I64x"
#define OCT64_FORMAT                            "%I64o"

//
#define GG_LONGLONG(x) x##LL
#define GG_ULONGLONG(x) x##ULL
// Per C99 7.8.14, define __STDC_CONSTANT_MACROS before including <stdint.h>
// to get the INTn_C and UINTn_C macros for integer constants.  It's difficult
// to guarantee any specific ordering of header includes, so it's difficult to
// guarantee that the INTn_C macros can be defined by including <stdint.h> at
// any specific point.  Provide GG_INTn_C macros instead.
#define GG_INT8_C(x)    (x)
#define GG_INT16_C(x)   (x)
#define GG_INT32_C(x)   (x)
#define GG_INT64_C(x)   GG_LONGLONG(x)

#define GG_UINT8_C(x)   (x ## U)
#define GG_UINT16_C(x)  (x ## U)
#define GG_UINT32_C(x)  (x ## U)
#define GG_UINT64_C(x)  GG_ULONGLONG(x)


#endif //__MACRO_X_BY_JADE_H_