#include <cassert>
#include <vector>
#include "Land.h"

namespace qtb
{
	Land::Land(const Area& area)
		: Zone(area, NULL)
		, m_nextBushID(0)
		, m_nextBushGroupID(0)
		, m_nextAreaID(0)
	{
	}

	Land::~Land()
	{
		clearStaticBush();
	}

	void Land::resetStaticBush(const AreaMap& areaMap)
	{
		clearStaticBush();
		generateBushMap(areaMap, m_staticBushMap);
	}

	void Land::clearStaticBush()
	{
		for (BushPMap::iterator it = m_staticBushMap.begin(); it != m_staticBushMap.end(); ++it)
		{
			assert(it->second);
			delete it->second;
		}
		m_staticBushMap.clear();
	}

	unsigned int Land::addDynamicArea(const Area& area)
	{
		unsigned int areaID = AllocAreaID();
		m_dynamicAreaMap[areaID] = area;
		return areaID;
	}
		
	void Land::removeDynamicArea(unsigned int id)
	{
		m_dynamicAreaMap.erase(id);
	}

	void Land::generateBushMap(const AreaMap& areaMap, BushPMap& bushMap)
	{
		bushMap.clear();

		for (AreaMap::const_iterator itArea = areaMap.begin(); itArea != areaMap.end(); ++itArea)
		{
			std::vector<unsigned> overlaps;
			for (BushPMap::iterator itBush = bushMap.begin(); itBush != bushMap.end(); ++itBush)
			{
				if (itBush->second->overlap(itArea->second))
					overlaps.push_back(itBush->first);
			}

			if (overlaps.empty())
			{
				Bush* bush = new Bush(this);
				bushMap[bush->id()] = bush;
				bush->addArea(itArea->second);
			}
			else
			{
				Bush* bush = bushMap[overlaps[0]];
				bush->addArea(itArea->second);
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

	void Land::rebuildBushGroup()
	{
		clearBushGroup();

		// static
		for (BushPMap::iterator itBush = m_staticBushMap.begin(); itBush != m_staticBushMap.end(); ++itBush)
		{
			BushGroup* group = new BushGroup(this);
			m_bushGroupMap[group->id()] = group;
			group->addBush(itBush->second);
		}

		// dynamic
		BushPMap dynamicBushMap;
		generateBushMap(m_dynamicAreaMap, dynamicBushMap);

		// splice
		for (BushPMap::const_iterator itBush = dynamicBushMap.begin(); itBush != dynamicBushMap.end(); ++itBush)
		{
			std::vector<unsigned> overlaps;
			for (BushGroupPMap::iterator itGroup = m_bushGroupMap.begin(); itGroup != m_bushGroupMap.end(); ++itGroup)
			{
				if (itGroup->second->overlap(*(itBush->second)))
					overlaps.push_back(itGroup->first);
			}

			if (overlaps.empty())
			{
				BushGroup* group = new BushGroup(this);
				m_bushGroupMap[group->id()] = group;
				group->addBush(itBush->second);
			}
			else
			{
				BushGroup* group = m_bushGroupMap[overlaps[0]];
				group->addBush(itBush->second);
				for (size_t i = 1; i < overlaps.size(); ++i)
				{
					BushGroup* spliceGroup = m_bushGroupMap[overlaps[i]];
					group->splice(*spliceGroup);
					m_bushGroupMap.erase(overlaps[i]);
					delete spliceGroup;
				}
			}
		}
	}

	void Land::clearBushGroup()
	{
		for (BushGroupPMap::iterator it = m_bushGroupMap.begin(); it != m_bushGroupMap.end(); ++it)
		{
			assert(it->second);
			delete it->second;
		}
		m_bushGroupMap.clear();
	}

}
