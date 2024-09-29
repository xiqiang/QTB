#include <cassert>
#include "BushGroup.h"
#include "Land.h"
#include "Util.h"

namespace qtb
{
	BushGroup::BushGroup(unsigned int id)
		: m_id(id)
		, m_zone(NULL)
	{
	}

	BushGroup::~BushGroup()
	{
	}

	bool BushGroup::overlap(const Bush& bush) const
	{
		if (!m_overall.overlap(bush.overall()))
			return false;

		BushPMap::const_iterator itEnd = m_bushes.end();
		for (BushPMap::const_iterator it = m_bushes.begin(); it != itEnd; ++it)
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

		BushPMap::const_iterator itEnd = m_bushes.end();
		for (BushPMap::const_iterator it = m_bushes.begin(); it != itEnd; ++it)
		{
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

	bool BushGroup::add(Bush* bush)
	{
		assert(bush);
		assert(m_bushes.find(bush->id()) == m_bushes.end());

		bool first = m_bushes.empty();
		try
		{
			QTB_RAND_BAD_ALLOC(1);
			std::pair<BushPMap::iterator, bool> ret = m_bushes.insert(
				std::pair<unsigned int, Bush*>(bush->id(), bush));
		}
		catch (std::bad_alloc&)
		{
			qtbLog("bad_alloc: BushGroup::add\n");
			return false;
		}

		const Area& bushArea = bush->overall();
		if (first)
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

		bush->m_group = this;
		return true;
	}

	void BushGroup::remove(unsigned int bushID)
	{
		BushPMap::iterator it = m_bushes.find(bushID);
		assert(it != m_bushes.end());
		Bush* bush = it->second;
		assert(bush);
		assert(bush->m_group == this);
		m_bushes.erase(it);
	}

	bool BushGroup::splice(BushGroup& other)
	{
		BushPMap::const_iterator itOtherBegin = other.m_bushes.begin();
		BushPMap::const_iterator itOtherEnd = other.m_bushes.end();

		try
		{
			QTB_RAND_BAD_ALLOC(1);
			m_bushes.insert(itOtherBegin, itOtherEnd);
		}
		catch (std::bad_alloc&)
		{
			qtbLog("bad_alloc: BushGroup::splice\n");
			return false;
		}

		const Area& otherOverall = other.overall();
		if (m_overall.left > otherOverall.left)
			m_overall.left = otherOverall.left;
		if (m_overall.right < otherOverall.right)
			m_overall.right = otherOverall.right;
		if (m_overall.bottom > otherOverall.bottom)
			m_overall.bottom = otherOverall.bottom;
		if (m_overall.top < otherOverall.top)
			m_overall.top = otherOverall.top;

		for (BushPMap::const_iterator it = itOtherBegin; it != itOtherEnd; ++it)
		{
			Bush* bush = it->second;
			assert(bush);
			assert(bush->m_group == &other);
			bush->m_group = this;
		}

		other.m_bushes.clear();
		return true;
	}

	void BushGroup::releaseBushes()
	{
		BushPMap::const_iterator itEnd = m_bushes.end();
		for (BushPMap::const_iterator it = m_bushes.begin(); it != itEnd; ++it)
		{
			Bush* bush = it->second;
			assert(bush->m_group == this);
			bush->m_group = NULL;
		}
	}

}

