#include <cassert>
#include "Zone.h"
#include "Util.h"

namespace qtb
{
	Zone::Zone(const Area& area,  Zone* parent /*= NULL*/)
		: QTree(area, parent)
		, m_childBindCount(0)
	{
	}

	QTree* Zone::newChild(const Area& area)
	{
		try
		{
			QTB_RAND_BAD_ALLOC(1);
			return new Zone(area, this);
		}
		catch (std::bad_alloc&)
		{
			qtbLog("bad_alloc:  Zone::newChild\n");
			return NULL;
		}
	}

	bool Zone::bushContains(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/)
	{
		if (!m_area.contains(x, y))
			return false;

		if(!m_boundBushGroups.empty() && _bushContains(x, y, bushGroupID, bushID))
			return true;

		if (m_childBindCount == 0)
			return false;

		Zone* c = dynamic_cast<Zone*>(child(x, y));
		if (c)
			return c->bushContains(x, y, bushGroupID, bushID);

		return false;
	}

	bool Zone::bushContainsRev(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/)
	{
		assert(m_area.contains(x, y));
		if (!m_boundBushGroups.empty() && _bushContains(x, y, bushGroupID, bushID))
			return true;

		Zone* parent = dynamic_cast<Zone*>(m_parent);
		if(parent)
			return parent->bushContainsRev(x, y, bushGroupID, bushID);

		return false;
	}

	bool Zone::bindBushGroup(BushGroup* group)
	{
		assert(group);
		assert(NULL == group->m_zone);
		assert(m_boundBushGroups.find(group->id()) == m_boundBushGroups.end());

		try
		{
			QTB_RAND_BAD_ALLOC(1);
			std::pair<BushGroupPMap::iterator, bool> ret = m_boundBushGroups.insert(
				std::pair<unsigned int, BushGroup*>(group->id(), group));
		}
		catch (std::bad_alloc&)
		{
			qtbLog("bad_alloc:  Zone::bindBushGroup\n");
			return false;
		}

		group->m_zone = this;
		if (m_parent)
		{
			Zone* parent = dynamic_cast<Zone*>(m_parent);
			assert(parent);
			parent->_incChildBindCount();
		}

		return true;
	}

	void Zone::unbindBushGroup(unsigned int groupID)
	{
		BushGroupPMap::iterator it = m_boundBushGroups.find(groupID);
		assert(it != m_boundBushGroups.end());
		assert(this == it->second->m_zone);

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
		BushGroupPMap::const_iterator itEnd = m_boundBushGroups.end();
		for (BushGroupPMap::const_iterator it = m_boundBushGroups.begin(); it != itEnd; ++it) {
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
