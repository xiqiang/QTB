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
		clearBushMap(m_dynamicBushMap);
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
		updateBushGroup();
		return areaID;
	}
		
	void Land::removeDynamicArea(unsigned int id)
	{
		m_dynamicAreaMap.erase(id);
		updateBushGroup();
	}

	void Land::setDynamicAreas(const AreaMap& areas)
	{
		m_dynamicAreaMap = areas;
		updateBushGroup();
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
			allocResideBushGroup(group);
		}
	}

	void Land::updateBushGroup()
	{
		// static
		rebuildStaticBushGroup();

		// dynamic
		clearBushMap(m_dynamicBushMap);
		generateBushMap(m_dynamicAreaMap, m_dynamicBushMap);

		// splice
		for (BushPMap::const_iterator itBush = m_dynamicBushMap.begin(); itBush != m_dynamicBushMap.end(); ++itBush)
		{
			std::vector<unsigned> overlaps;

			Zone* zone = this;
			while (zone)
			{
				const BushGroupPMap& resideBushGroupMap = zone->getResideBushGroup();
				for (BushGroupPMap::const_iterator itGroup = resideBushGroupMap.begin(); itGroup != resideBushGroupMap.end(); ++itGroup)
				{
					if (itGroup->second->overlap(*(itBush->second)))
						overlaps.push_back(itGroup->first);
				}

				const Area& bushArea  = itBush->second->overall();
				QTree* tree = zone->getChild(bushArea.x(), bushArea.y());
				if (tree && tree->area().contains(bushArea))
					zone = dynamic_cast<Zone*>(tree);
				else
					zone = NULL;
			}

			if (overlaps.empty())
			{
				BushGroup* group = new BushGroup(this);
				m_bushGroupMap[group->id()] = group;
				group->addBush(itBush->second);
				allocResideBushGroup(group);
			}
			else
			{
				BushGroup* group = m_bushGroupMap[overlaps[0]];
				assert(group->zone());
				group->zone()->removeResideBushGroup(overlaps[0]);
				group->addBush(itBush->second);

				for (size_t i = 1; i < overlaps.size(); ++i)
				{
					BushGroup* spliceGroup = m_bushGroupMap[overlaps[i]];
					assert(spliceGroup->zone());
					spliceGroup->zone()->removeResideBushGroup(overlaps[i]);

					group->splice(*spliceGroup);
					m_bushGroupMap.erase(overlaps[i]);
					delete spliceGroup;
				}

				allocResideBushGroup(group);
			}
		}
	}

	void Land::allocResideBushGroup(BushGroup* group)
	{
		assert(group->zone() == NULL);
		Zone* zone = dynamic_cast<Zone*>(locateTree(group->overall()));
		assert(zone);
		zone->addResideBushGroup(group);
	}
}
