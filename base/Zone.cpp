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

	bool Zone::bushContains(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/)
	{
		if (!m_area.contains(x, y))
			return false;

		if(_bushContains(x, y, bushGroupID, bushID))
			return true;

		Zone* c = dynamic_cast<Zone*>(child(x, y));
		if (c)
			return c->bushContains(x, y, bushGroupID, bushID);

		return false;
	}

	void Zone::addBushGroup(BushGroup* group)
	{
		assert(group);
		assert(NULL == group->m_zone);

		group->m_zone = this;
		assert(m_bushGroups.find(group->id()) == m_bushGroups.end());
		m_bushGroups[group->id()] = group;
	}

	void Zone::removeBushGroup(unsigned int groupID)
	{
		BushGroupPMap::iterator it = m_bushGroups.find(groupID);
		assert(it != m_bushGroups.end());
		it->second->m_zone = NULL;
		m_bushGroups.erase(groupID);
	}

	bool Zone::_bushContains(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_area.contains(x, y))
			return false;

		for (BushGroupPMap::const_iterator it = m_bushGroups.begin(); it != m_bushGroups.end(); ++it) {
			BushGroup* group = it->second;
			assert(group);

			if (group->contains(x, y, bushID)) {
				if (bushGroupID)
					*bushGroupID = group->id();
				return true;
			}
		}

		return false;
	}

}
