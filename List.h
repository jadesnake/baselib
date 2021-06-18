#pragma once

#include <algorithm>
#include <vector>
#include <memory>
namespace base
{
	class Locker
	{
	public:
		void Lock(){}
		void UnLock(){}
	};
	//std::tr1::shared_ptr 管理的指针地址会变化
	//
	template< typename theItem,class theLock=Locker >
	class IListImpl
	{
	public:
		IListImpl(unsigned int nType = 0) : m_unType(nType)
		{	m_nSz = 0;		};
		~IListImpl()
		{	clear();		};
		std::tr1::shared_ptr<theItem> push_back()
		{
			std::tr1::shared_ptr<theItem> ret( new theItem() );
			m_lock.Lock();
			m_lstC.push_back(ret);
			m_lock.UnLock();
			return ret;
		}
		void push_back(std::tr1::shared_ptr<theItem> one)
		{
			m_lock.Lock();
			m_lstC.push_back(one);
			m_lock.UnLock();
		}
		void assign( IListImpl<theItem,theLock> &inCopy)
		{
			m_lock.Lock();
			inCopy.m_lock.Lock();

			m_lstC.assign(inCopy.m_lstC.begin(),inCopy.m_lstC.end());

			inCopy.m_lock.UnLock();
			m_lock.UnLock();
		}
		void takeAll(IListImpl<theItem,theLock> &inCopy)
		{
			m_lock.Lock();
			inCopy.m_lock.Lock();

			m_lstC.assign(inCopy.m_lstC.begin(),inCopy.m_lstC.end());
			inCopy.clear();
			
			inCopy.m_lock.UnLock();
			m_lock.UnLock();
		}
		std::tr1::shared_ptr<theItem>	getLast() 
		{
			std::tr1::shared_ptr<theItem> ret;
			m_lock.Lock();
			if (m_lstC.empty())
			{
				m_lock.UnLock();
				return ret;
			}
			ret = m_lstC.back();
			m_lock.UnLock();
			return ret;
		}
		std::tr1::shared_ptr<theItem>	getBegin()
		{
			std::tr1::shared_ptr<theItem> ret;
			m_lock.Lock();
			if( m_lstC.empty() )
			{
				m_lock.UnLock();
				return ret;
			}
			m_nSz = 0;
			ret = m_lstC[m_nSz];
			m_lock.UnLock();
			return ret;
		}
		std::tr1::shared_ptr<theItem>	 getNext()
		{
			std::tr1::shared_ptr<theItem> ret;
			m_lock.Lock();
			if( m_nSz < m_lstC.size() )
			{
				ret = m_lstC[m_nSz];
				m_nSz++;
			}
			m_lock.UnLock();
			return ret;
		}
		std::tr1::shared_ptr<theItem>  at(unsigned int unV)
		{
			std::tr1::shared_ptr<theItem> ret;
			m_lock.Lock();
			if( 0 == m_lstC.size() )
			{
				m_lock.UnLock();
				return ret;
			}
			if(unV < m_lstC.size())
			{
				ret = m_lstC.at(unV);
				m_lock.UnLock();
			}
			return ret;
		}
		std::tr1::shared_ptr<theItem>  operator[](unsigned int unV)
		{
			return at(unV);
		}
		void	reset()
		{	
			m_lock.Lock();
			m_nSz = 0;	
			m_lock.UnLock();
		}
		unsigned int count()
		{
			m_lock.Lock();
			unsigned int sz = m_lstC.size();	
			m_lock.UnLock();
			return sz;
		}
		unsigned int erase(std::tr1::weak_ptr<theItem> one)
		{
			unsigned int unR = 0;
			if (one.expired()) 
			{
				m_lock.Lock();
				unR = m_lstC.size();
				m_lock.UnLock();
				return unR;
			}
			m_lock.Lock();
			std::vector<std::tr1::shared_ptr<theItem>>::iterator it = std::find(m_lstC.begin(),m_lstC.end(), one.lock());
			if( it != m_lstC.end() )
			{
				if( m_nSz )
				{
					size_t nT = (it - m_lstC.begin());
					if( m_nSz >= nT )
						m_nSz--;
				}
				m_lstC.erase(it);
			}
			unR = m_lstC.size();
			m_lock.UnLock();
			return unR;
		}
		void clear()
		{
			m_lock.Lock();
			m_lstC.clear();				
			m_nSz = 0;
			m_lock.UnLock();
		}
		void swap( IListImpl<theItem, theLock> &v ) {		
			m_lock.Lock();
				v.m_lock.Lock();
					m_lstC.swap(v.m_lstC);
					m_nSz = 0;
					v.m_nSz = 0;
				v.m_lock.UnLock();
			m_lock.UnLock();
		}
		void  release()
		{	delete this;		}
		unsigned int getType(void)
		{	return m_unType;	}
	private:
		std::vector<std::tr1::shared_ptr<theItem>> m_lstC;
		size_t				  m_nSz;
		unsigned int		  m_unType;
		theLock               m_lock;
	};
};