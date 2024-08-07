#include <cassert>
#include "Zone.h"

namespace qtb
{
	Zone::Zone(const Area& area,  Zone* parent /*= NULL*/)
		: QTree(area, parent)
	{
		if (parent)
			m_generation = parent->generation() + 1;
		else
			m_generation = 0;
	}

	QTree* Zone::newChild(const Area& area)
	{
		return new Zone(area, this);
	}

	unsigned int Zone::crossBushGroupID(float x, float y)
	{
		if (!m_area.contains(x, y))
			return -1;

		unsigned int bushID = -1;
		if(bushCross(x, y, &bushID))
			return bushID;

		Zone* child = dynamic_cast<Zone*>(getChild(x, y));
		if (child)
			return child->crossBushGroupID(x, y);

		return -1;
	}

	void Zone::addResideBushGroup(BushGroup* group)
	{
		assert(group);
		assert(NULL == group->m_zone);

		group->m_zone = this;
		assert(m_resideBushGroupMap.find(group->id()) == m_resideBushGroupMap.end());
		m_resideBushGroupMap[group->id()] = group;
	}

	void Zone::removeResideBushGroup(BushGroup* group)
	{
		assert(group->m_zone == this);
		group->m_zone = NULL;
		m_resideBushGroupMap.erase(group->id());
	}

	void Zone::removeResideBushGroup(unsigned int groupID)
	{
		BushGroupPMap::iterator it = m_resideBushGroupMap.find(groupID);
		assert(it != m_resideBushGroupMap.end());
		it->second->m_zone = NULL;
		m_resideBushGroupMap.erase(groupID);
	}

	bool Zone::bushCross(float x, float y, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_area.contains(x, y))
			return false;

		for (BushGroupPMap::const_iterator it = m_resideBushGroupMap.begin(); it != m_resideBushGroupMap.end(); ++it) {
			BushGroup* group = it->second;
			assert(group);

			if (group->bushCheck(x, y)) {
				if (bushID)
					*bushID = group->id();
				return true;
			}
		}

		return false;
	}

}
