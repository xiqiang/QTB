#include <cassert>
#include "Bunch.h"
#include "Land.h"

namespace qtb
{
	Bunch::Bunch(Land* land)
	{
		assert(land);
		m_land = land;
		m_id = land->AllocBunchID();
		m_zone = NULL;
	}

	void Bunch::addBush(Bush* bush)
	{
		assert(bush);
		const Area& bushArea = bush->overall();
		m_overall.left = m_overall.left < bushArea.left ? m_overall.left : bushArea.left;
		m_overall.right = m_overall.right > bushArea.right ? m_overall.right : bushArea.right;
		m_overall.bottom = m_overall.bottom < bushArea.bottom ? m_overall.bottom : bushArea.bottom;
		m_overall.top = m_overall.top > bushArea.top ? m_overall.top : bushArea.top;
		m_bushList.push_back(bush);
	}

	bool Bunch::bushCheck(float x, float y) const
	{
		if (!m_overall.cross(x, y))
			return false;

		for (BushPList::const_iterator it = m_bushList.begin(); it != m_bushList.end(); ++it) {
			const Bush* bush = *it;
			assert(bush);
			if (bush->cross(x, y))
				return true;
		}
		return false;
	}
}

