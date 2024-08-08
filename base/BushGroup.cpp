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
		const Area& bushArea = bush->overall();

		if (m_bushList.empty())
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

		m_bushList.push_back(bush);
		bush->m_group = this;
	}

	bool BushGroup::overlap(const Bush& bush) const
	{
		if (!m_overall.overlap(bush.overall()))
			return false;

		for (BushPList::const_iterator it = m_bushList.begin(); it != m_bushList.end(); ++it)
		{
			if ((*it)->overlap(bush))
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

		for (BushPList::iterator it = r.m_bushList.begin(); it != r.m_bushList.end(); ++it)
			(*it)->m_group = this;

		m_bushList.splice(m_bushList.end(), r.m_bushList);
		r.m_bushList.clear();
	}

	bool BushGroup::bushCheck(float x, float y, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_overall.contains(x, y))
			return false;

		for (BushPList::const_iterator it = m_bushList.begin(); it != m_bushList.end(); ++it) {
			const Bush* bush = *it;
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

