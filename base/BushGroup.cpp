#include <cassert>
#include "BushGroup.h"
#include "Land.h"

namespace qtb
{
	BushGroup::BushGroup(unsigned int id)
		: m_id(id)
		, m_zone(NULL)
	{
	}

	bool BushGroup::overlap(const Bush& bush) const
	{
		if (!m_overall.overlap(bush.overall()))
			return false;

		for (BushPMap::const_iterator it = m_bushes.begin(); it != m_bushes.end(); ++it)
		{
			assert(it->second);
			if (it->second->overlap(bush))
				return true;
		}
		return false;
	}

	bool BushGroup::contains(float x, float y, unsigned int* bushID /*= NULL*/) const
	{
		if (!m_overall.contains(x, y))
			return false;

		for (BushPMap::const_iterator it = m_bushes.begin(); it != m_bushes.end(); ++it) {
			const Bush* bush = it->second;
			assert(bush);
			if (bush->contains(x, y))
			{
				if (bushID)
					*bushID = bush->id();
				return true;
			}
		}
		return false;
	}

	void BushGroup::add(Bush* bush)
	{
		assert(bush);
		assert(m_bushes.find(bush->id()) == m_bushes.end());

		const Area& bushArea = bush->overall();

		if (m_bushes.empty())
		{
			m_overall = bushArea;
		}
		else
		{
			if (m_overall.left > bushArea.left)
				m_overall.left = bushArea.left;
			if (m_overall.right < bushArea.right)
				m_overall.right = bushArea.right;
			if (m_overall.bottom > bushArea.bottom)
				m_overall.bottom = bushArea.bottom;
			if (m_overall.top < bushArea.top)
				m_overall.top = bushArea.top;
		}

		m_bushes[bush->id()] = bush;
		bush->m_group = this;
	}

	void BushGroup::remove(unsigned int bushID)
	{
		BushPMap::iterator it = m_bushes.find(bushID);
		assert(it != m_bushes.end());
		assert(it->second);
		assert(it->second->group() == this);
		m_bushes.erase(it);
	}

	void BushGroup::splice(BushGroup& other)
	{
		const Area& area = other.overall();
		if (m_overall.left > area.left)
			m_overall.left = area.left;
		if (m_overall.right < area.right)
			m_overall.right = area.right;
		if (m_overall.bottom > area.bottom)
			m_overall.bottom = area.bottom;
		if (m_overall.top < area.top)
			m_overall.top = area.top;

		for (BushPMap::iterator it = other.m_bushes.begin(); it != other.m_bushes.end(); ++it)
		{
			assert(it->second);
			assert(it->second->group() == &other);
			it->second->m_group = this;
		}

		m_bushes.insert(other.m_bushes.begin(), other.m_bushes.end());
		other.m_bushes.clear();
	}

}

