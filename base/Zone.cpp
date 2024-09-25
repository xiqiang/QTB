#include <cassert>
#include "Zone.h"

namespace qtb
{
	Zone::Zone(const Area& area,  Zone* parent /*= NULL*/)
		: QTree(area, parent)
		, m_childBindCount(0)
	{
	}

	QTree* Zone::newChild(const Area& area)
	{
		return new Zone(area, this);
	}

	bool Zone::bushContains(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/)
	{
		if(m_boundBushGroups.size() > 0 && _bushContains(x, y, bushGroupID, bushID))
			return true;

		if (m_childBindCount == 0)
			return false;

		Zone* c = dynamic_cast<Zone*>(child(x, y));
		if (c)
			return c->bushContains(x, y, bushGroupID, bushID);

		return false;
	}

	void Zone::bindBushGroup(BushGroup* group)
	{
		assert(group);
		assert(NULL == group->m_zone);

		group->m_zone = this;
		assert(m_boundBushGroups.find(group->id()) == m_boundBushGroups.end());
		m_boundBushGroups[group->id()] = group;

		if (m_parent)
		{
			Zone* parent = dynamic_cast<Zone*>(m_parent);
			assert(parent);
			parent->_incChildBindCount();
		}
	}

	void Zone::unbindBushGroup(unsigned int groupID)
	{
		BushGroupPMap::iterator it = m_boundBushGroups.find(groupID);
		assert(it != m_boundBushGroups.end());
		it->second->m_zone = NULL;
		m_boundBushGroups.erase(groupID);

		if (m_parent)
		{
			Zone* parent = dynamic_cast<Zone*>(m_parent);
			assert(parent);
			parent->_decChildBindCount();
		}
	}

	bool Zone::_bushContains(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_area.contains(x, y))
			return false;

		for (BushGroupPMap::const_iterator it = m_boundBushGroups.begin(); it != m_boundBushGroups.end(); ++it) {
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

	void Zone::_incChildBindCount()
	{
		++m_childBindCount; 

		if (m_parent)
		{
			Zone* parent = dynamic_cast<Zone*>(m_parent);
			assert(parent);
			parent->_incChildBindCount();
		}
	}

	void Zone::_decChildBindCount()
	{ 
		assert(m_childBindCount > 0);
		--m_childBindCount;

		if (m_parent)
		{
			Zone* parent = dynamic_cast<Zone*>(m_parent);
			assert(parent);
			parent->_decChildBindCount();
		}
	}
}
