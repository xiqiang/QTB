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
		clearBushMap(m_staticBushMap);
		clearBushGroup();
	}

	void Land::resetStaticBush(const AreaMap& areaMap)
	{
		clearBushMap(m_staticBushMap);
		generateBushMap(areaMap, m_staticBushMap);
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

	void Land::setDynamicAreas(const AreaMap& areas)
	{
		m_dynamicAreaMap = areas;
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

	void Land::clearBushMap(BushPMap& bushMap)
	{
		for (BushPMap::iterator it = bushMap.begin(); it != bushMap.end(); ++it)
		{
			assert(it->second);
			delete it->second;
		}
		bushMap.clear();
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

		// clear
		clearBushMap(dynamicBushMap);
	}

	void Land::clearBushGroup()
	{
		for (BushGroupPMap::iterator it = m_bushGroupMap.begin(); it != m_bushGroupMap.end(); ++it)
		{
			BushGroup* group = it->second;
			assert(group);
			if (group->zone())
				group->zone()->removeResideBushGroup(group);
			delete group;
		}
		m_bushGroupMap.clear();
	}

	void Land::rebuildStaticBushGroup()
	{
		clearBushGroup();

		// static
		for (BushPMap::iterator itBush = m_staticBushMap.begin(); itBush != m_staticBushMap.end(); ++itBush)
		{
			BushGroup* group = new BushGroup(this);
			m_bushGroupMap[group->id()] = group;
			group->addBush(itBush->second);

			Zone* zone = dynamic_cast<Zone*>(locateTree(group->overall()));
			assert(zone);
			zone->addResideBushGroup(group);
		}
	}

}
