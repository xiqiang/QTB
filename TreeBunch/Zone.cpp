#include <cassert>
#include "Zone.h"

namespace treebush
{
	Zone::Zone(const Area& area,  Zone* parent /*= NULL*/)
		: QTree(area, parent)
	{
	}

	QTree* Zone::newChild(const Area& area)
	{
		return new Zone(area, this);
	}
	
	void Zone::addBunch(Bunch* bunch)
	{
		assert(bunch);
		assert(NULL == bunch->m_zone);

		bunch->m_zone = this;
		assert(m_bunchMap.find(bunch->id()) == m_bunchMap.end());
		m_bunchMap[bunch->id()] = bunch;
	}

	bool Zone::bushCross(float x, float y, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_area.cross(x, y))
			return false;

		for (BunchMap::const_iterator it = m_bunchMap.begin(); it != m_bunchMap.end(); ++it) {
			Bunch* bunch = it->second;
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
