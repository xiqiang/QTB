#include <cassert>
#include "BushGroup.h"
#include "Land.h"

namespace qtb
{
	BushGroup::BushGroup(Land* land)
	{
		assert(land);
		m_land = land;
		m_id = land->AllocBushGroupID();
		m_zone = NULL;
	}

	void BushGroup::addBush(Bush* bush)
	{
		assert(bush);
		assert(m_bushMap.find(bush->id()) == m_bushMap.end());

		const Area& bushArea = bush->overall();

		if (m_bushMap.empty())
		{
			m_overall = bushArea;
		}
		else
		{
			m_overall.left = m_overall.left < bushArea.left ? m_overall.left : bushArea.left;
			m_overall.right = m_overall.right > bushArea.right ? m_overall.right : bushArea.right;
			m_overall.bottom = m_overall.bottom < bushArea.bottom ? m_overall.bottom : bushArea.bottom;
			m_overall.top = m_overall.top > bushArea.top ? m_overall.top : bushArea.top;
		}

		m_bushMap[bush->id()] = bush;
		bush->m_group = this;
	}

	void BushGroup::removeBush(unsigned int bushID)
	{
		assert(m_bushMap.find(bushID) != m_bushMap.end());
		m_bushMap.erase(bushID);
	}

	bool BushGroup::overlap(const Bush& bush) const
	{
		if (!m_overall.overlap(bush.overall()))
			return false;

		for (BushPMap::const_iterator it = m_bushMap.begin(); it != m_bushMap.end(); ++it)
		{
			assert(it->second);

			if (it->second->overlap(bush))
				return true;
		}
		return false;
	}

	void BushGroup::splice(BushGroup& r)
	{
		const Area& area = r.overall();
		m_overall.left = m_overall.left < area.left ? m_overall.left : area.left;
		m_overall.right = m_overall.right > area.right ? m_overall.right : area.right;
		m_overall.bottom = m_overall.bottom < area.bottom ? m_overall.bottom : area.bottom;
		m_overall.top = m_overall.top > area.top ? m_overall.top : area.top;

		for (BushPMap::iterator it = r.m_bushMap.begin(); it != r.m_bushMap.end(); ++it)
		{
			assert(it->second);
			it->second->m_group = this;
		}

		m_bushMap.insert(r.m_bushMap.begin(), r.m_bushMap.end());
		r.m_bushMap.clear();
	}

	bool BushGroup::bushCheck(float x, float y, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_overall.contains(x, y))
			return false;

		for (BushPMap::const_iterator it = m_bushMap.begin(); it != m_bushMap.end(); ++it) {
			const Bush* bush = it->second;
			assert(bush);
			if (bush->cross(x, y))
			{
				if (bushID)
					*bushID = bush->id();
				return true;
			}
		}
		return false;
	}
}

