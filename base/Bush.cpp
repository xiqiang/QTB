#include <cassert>
#include "Bush.h"
#include "Land.h"
#include "Debug.h"

namespace qtb
{
	Bush::Bush(unsigned int id)
		: m_id(id)
		, m_group(NULL)
		, m_singleton(true)
		, m_isStatic(false)
	{		
	}

	bool Bush::overlap(const Bush& other) const
	{
		if (!m_overall.overlap(other.m_overall))
			return false;

		if (m_singleton && other.m_singleton)
			return true;

		AreaList::const_iterator itEnd = m_areaList.end();
		for (AreaList::const_iterator it = m_areaList.begin(); it != itEnd; ++it)
		{
			if (other.overlap(*it))
				return true;
		}
		return false;
	}

	bool Bush::overlap(const Area& area) const
	{
		if (!m_overall.overlap(area))
			return false;

		if (m_singleton)
			return true;

		AreaList::const_iterator itEnd = m_areaList.end();
		for (AreaList::const_iterator it = m_areaList.begin(); it != itEnd; ++it)
		{
			if (it->overlap(area))
				return true;
		}
		return false;
	}

	bool Bush::contains(float x, float y) const
	{
		if (!m_overall.contains(x, y))
			return false;

		if (m_singleton)
			return true;

		AreaList::const_iterator itEnd = m_areaList.end();
		for (AreaList::const_iterator it = m_areaList.begin(); it != itEnd; ++it)
		{
			if (it->contains(x, y))
				return true;
		}
		return false;
	}

	bool Bush::add(const Area& area)
	{
		bool first = m_areaList.empty();
		try
		{
			QTB_RAND_BAD_ALLOC(1);
			m_areaList.push_back(area);
		}
		catch (std::bad_alloc&)
		{
			qtbLog("bad_alloc: Bush::add\n");
			return false;
		}

		if (first)
		{
			m_overall = area;
		}
		else
		{
			if (m_overall.left > area.left)
				m_overall.left = area.left;
			if(m_overall.right < area.right)
				m_overall.right = area.right;
			if( m_overall.bottom > area.bottom)
				m_overall.bottom = area.bottom;
			if(m_overall.top < area.top)
				m_overall.top = area.top;
			m_singleton = false;
		}

		return true;
	}

	bool Bush::splice(Bush& other)
	{
		try
		{
			QTB_RAND_BAD_ALLOC(1);
			m_areaList.splice(m_areaList.end(), other.m_areaList);
			m_singleton = false;
		}
		catch (std::bad_alloc&)
		{
			qtbLog("bad_alloc: Bush::splice\n");
			return false;
		}

		const Area& otherOverall = other.overall();
		if (m_overall.left > otherOverall.left)
			m_overall.left = otherOverall.left;
		if (m_overall.right < otherOverall.right)
			m_overall.right = otherOverall.right;
		if (m_overall.bottom > otherOverall.bottom)
			m_overall.bottom = otherOverall.bottom;
		if (m_overall.top < otherOverall.top)
			m_overall.top = otherOverall.top;

		other.m_areaList.clear();
		return true;
	}

}

