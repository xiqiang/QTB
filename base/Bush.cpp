#include <cassert>
#include "Bush.h"
#include "Land.h"
#include "Util.h"

namespace qtb
{
	Bush::Bush(unsigned int id)
		: m_id(id)
		, m_group(NULL)
		, m_isStatic(false)
	{		
	}

	bool Bush::overlap(const Bush& other) const
	{
		if (!m_overall.overlap(other.m_overall))
			return false;

		size_t size = m_areaList.size();
		size_t size_r = other.m_areaList.size();

		AreaList::const_iterator itEnd = m_areaList.end();
		for (AreaList::const_iterator it = m_areaList.begin(); it != itEnd; ++it)
		{
			if (size > 1 && size_r > 1 && !it->overlap(other.m_overall))
				continue;

			AreaList::const_iterator it_rEnd = other.m_areaList.end();
			for (AreaList::const_iterator it_r = other.m_areaList.begin(); it_r != it_rEnd; ++it_r)
			{
				if (it->overlap(*it_r))
					return true;
			}
		}
		return false;
	}

	bool Bush::overlap(const Area& area) const
	{
		if (!m_overall.overlap(area))
			return false;

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
		}

		return true;
	}

	bool Bush::splice(Bush& other)
	{
		try
		{
			QTB_RAND_BAD_ALLOC(1);
			m_areaList.splice(m_areaList.end(), other.m_areaList);
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

