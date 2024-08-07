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
			BushGroup* bunch = it->second;
			assert(bunch);

			if (bunch->bushCheck(x, y)) {
				if (bushID)
					*bushID = bunch->id();
				return true;
			}
		}

		return false;
	}
}
