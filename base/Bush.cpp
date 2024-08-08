#include <cassert>
#include "Bush.h"
#include "Land.h"

namespace qtb
{
	Bush::Bush(Land* land)
	{
		assert(land);
		m_land = land;
		m_id = land->AllocBushID();
	}

	void Bush::addArea(const Area& area)
	{
		if (m_areaList.empty())
		{
			m_overall = area;
		}
		else
		{
			m_overall.left = m_overall.left < area.left ? m_overall.left : area.left;
			m_overall.right = m_overall.right > area.right ? m_overall.right : area.right;
			m_overall.bottom = m_overall.bottom < area.bottom ? m_overall.bottom : area.bottom;
			m_overall.top = m_overall.top > area.top ? m_overall.top : area.top;
		}

		m_areaList.push_back(area);
	}

	void Bush::splice(Bush& r)
	{
		const Area& area = r.overall();
		m_overall.left = m_overall.left < area.left ? m_overall.left : area.left;
		m_overall.right = m_overall.right > area.right ? m_overall.right : area.right;
		m_overall.bottom = m_overall.bottom < area.bottom ? m_overall.bottom : area.bottom;
		m_overall.top = m_overall.top > area.top ? m_overall.top : area.top;

		m_areaList.splice(m_areaList.end(), r.m_areaList);
		r.m_areaList.clear();
	}

	bool Bush::overlap(const Bush& r) const
	{
		if (!m_overall.overlap(r.m_overall))
			return false;

		size_t size = m_areaList.size();
		size_t size_r = r.m_areaList.size();

		for (AreaList::const_iterator it = m_areaList.begin(); it != m_areaList.end(); ++it)
		{
			if (size > 1 && size_r > 1 && !it->overlap(r.m_overall))
				continue;

			for (AreaList::const_iterator it_r = r.m_areaList.begin(); it_r != r.m_areaList.end(); ++it_r)
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

	bool Bush::cross(float x, float y) const
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

}

