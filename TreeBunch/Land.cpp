#include "Land.h"

namespace treebush
{
	Land::Land(const Area& area, float minZoneSize)
		: Zone(area, minZoneSize, NULL)
		, m_nextBushID(0)
		, m_nextBunchID(0)
	{
	}

	void Land::resetStaticBush(const AreaList& areaList)
	{
		if (areaList.empty())
			return;

		BushPMap bushMap;

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
				for(size_t o = 1; o < overlaps.size(); ++o)

			}
		}
	}
}
