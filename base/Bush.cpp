#include <cassert>
#include "Bush.h"
#include "Land.h"

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

		for (AreaList::const_iterator it = m_areaList.begin(); it != m_areaList.end(); ++it)
		{
			if (size > 1 && size_r > 1 && !it->overlap(other.m_overall))
				continue;

			for (AreaList::const_iterator it_r = other.m_areaList.begin(); it_r != other.m_areaList.end(); ++it_r)
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

		for (AreaList::const_iterator it = m_areaList.begin(); it != m_areaList.end(); ++it)
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

		for (AreaList::const_iterator it = m_areaList.begin(); it != m_areaList.end(); ++it)
		{
			if (it->contains(x, y))
				return true;
		}
		return false;
	}

	void Bush::add(const Area& area)
	{
		if (m_areaList.empty())
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

		m_areaList.push_back(area);
	}

	void Bush::splice(Bush& other)
	{
		const Area& area = other.overall();
		if (m_overall.left > area.left)
			m_overall.left = area.left;
		if (m_overall.right < area.right)
			m_overall.right = area.right;
		if (m_overall.bottom > area.bottom)
			m_overall.bottom = area.bottom;
		if (m_overall.top < area.top)
			m_overall.top = area.top;

		m_areaList.splice(m_areaList.end(), other.m_areaList);
		other.m_areaList.clear();
	}

}

