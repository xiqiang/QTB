#include <cassert>
#include "Land.h"

namespace qtb
{
	Land::Land(const Area& area)
		: Zone(area, NULL)
		, m_nextBushID(0)
		, m_nextBunchID(0)
	{
	}

	Land::~Land()
	{
		clearStaticBush();
	}

	void Land::resetStaticBush(const AreaList& areaList)
	{
		clearStaticBush();
		generateBushMap(areaList, m_staticBush);
	}

	void Land::generateBushMap(const AreaList& areaList, BushPMap& bushMap)
	{
		bushMap.clear();

		if (areaList.empty())
			return;

		for (AreaList::const_iterator itArea = areaList.begin(); itArea != areaList.end(); ++itArea)
		{
			std::vector<unsigned> overlaps;
			for(BushPMap::iterator itBush = bushMap.begin(); itBush != bushMap.end(); ++itBush)
			{
				if (itBush->second->overlap(*itArea))
					overlaps.push_back(itBush->first);
			}

			if (overlaps.empty())
			{
				Bush* bush = new Bush(this, true);
				bushMap[bush->id()] = bush;
				bush->addArea(*itArea);
			}
			else
			{
				Bush* bush = bushMap[overlaps[0]];
				bush->addArea(*itArea);
				for (size_t i = 1; i < overlaps.size(); ++i)
				{
					Bush* spliceBush = bushMap[overlaps[i]];
					bush->splice(*spliceBush);
					bushMap.erase(overlaps[i]);
					delete spliceBush;
				}
			}
		}
	}

	void Land::clearStaticBush()
	{
		for (BushPMap::iterator it = m_staticBush.begin(); it != m_staticBush.end(); ++it)
		{
			assert(it->second);
			delete it->second;
		}
		m_staticBush.clear();
	}

}
