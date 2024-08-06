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
	
	void Zone::addBushGroup(BushGroup* bunch)
	{
		assert(bunch);
		assert(NULL == bunch->m_zone);

		bunch->m_zone = this;
		assert(m_bushGroupMap.find(bunch->id()) == m_bushGroupMap.end());
		m_bushGroupMap[bunch->id()] = bunch;
	}

	bool Zone::bushCross(float x, float y, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_area.contains(x, y))
			return false;

		for (BushGroupPMap::const_iterator it = m_bushGroupMap.begin(); it != m_bushGroupMap.end(); ++it) {
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
