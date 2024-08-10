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
		clear();
	}

	void Land::clear()
	{
		clearBushMap(m_staticBushMap);
		clearBushMap(m_dynamicBushMap);
		clearBushGroup();
	}

	void Land::rebuild(const AreaMap& areaMap)
	{
		clear();

		generateBushMap(areaMap, m_staticBushMap);
		for (BushPMap::iterator itBush = m_staticBushMap.begin(); itBush != m_staticBushMap.end(); ++itBush)
		{
			BushGroup* group = new BushGroup(this);
			m_bushGroupMap[group->id()] = group;
			group->addBush(itBush->second);
			resideBushGroup(group);
		}
	}

	unsigned int Land::createDynamicBush(const Area& area, Area* influence /*= NULL*/)
	{
		Bush* bush = new Bush(this);
		bush->addArea(area);
		m_dynamicBushMap[bush->id()] = bush;

		BushGroup* group = resideBush(bush);
		assert(group);

		if (influence)
			*influence = group->overall();

		return bush->id();
	}

	bool Land::removeDynamicBush(unsigned int id, Area* influence /*= NULL*/)
	{
		BushPMap::iterator itFind = m_dynamicBushMap.find(id);
		if (m_dynamicBushMap.end() == itFind)
			return false;

		Bush* bush = itFind->second;
		assert(bush);
		BushGroup* group = bush->group();
		assert(group);

		if (influence)
			*influence = group->overall();

		assert(group->zone());
		group->zone()->removeResideBushGroup(group->id());

		group->removeBush(id);
		recycleBushGroup(group);

		m_dynamicBushMap.erase(id);
		delete bush;

		return true;
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
					assert(spliceBush);
					bush->splice(*spliceBush);

					bushMap.erase(overlaps[i]);
					delete spliceBush;
				}
			}
		}
	}

	void Land::recycleBushGroup(BushGroup* group)
	{
		assert(group);
		assert(m_bushGroupMap.find(group->id()) != m_bushGroupMap.end());
		BushGroupPMap groupMap;

		const BushPMap& bushMap = group->bushMap();
		for (BushPMap::const_iterator itBush = bushMap.begin(); itBush != bushMap.end(); ++itBush)
		{
			std::vector<unsigned> overlaps;
			for (BushGroupPMap::iterator itGroup = groupMap.begin(); itGroup != groupMap.end(); ++itGroup)
			{
				if (itGroup->second->overlap(*(itBush->second)))
					overlaps.push_back(itGroup->first);
			}

			if (overlaps.empty())
			{
				BushGroup* cacheGroup = new BushGroup(this);
				groupMap[cacheGroup->id()] = cacheGroup;
				cacheGroup->addBush(itBush->second);
			}
			else
			{
				BushGroup* cacheGroup = groupMap[overlaps[0]];
				cacheGroup->addBush(itBush->second);
				for (size_t i = 1; i < overlaps.size(); ++i)
				{
					BushGroup* spliceGroup = groupMap[overlaps[i]];
					assert(spliceGroup);
					cacheGroup->splice(*spliceGroup);

					groupMap.erase(overlaps[i]);
					delete spliceGroup;
				}
			}
		}

		for (BushGroupPMap::const_iterator itGroup = groupMap.begin(); itGroup != groupMap.end(); ++itGroup)
			resideBushGroup(itGroup->second);

		m_bushGroupMap.insert(groupMap.begin(), groupMap.end());
		m_bushGroupMap.erase(group->id());
		delete group;
	}

	BushGroup* Land::resideBush(Bush* bush)
	{
		assert(bush);

		std::vector<unsigned> overlaps;
		std::list<QTree*> treeList;
		getOerlapTrees(bush->overall(), treeList);

		for (std::list<QTree*>::iterator itTree = treeList.begin(); itTree != treeList.end(); ++itTree)
		{
			Zone* zone = dynamic_cast<Zone*>(*itTree);
			assert(zone);

			const BushGroupPMap& resideBushGroupMap = zone->getResideBushGroup();
			for (BushGroupPMap::const_iterator itGroup = resideBushGroupMap.begin(); itGroup != resideBushGroupMap.end(); ++itGroup)
			{
				if (itGroup->second->overlap(*(bush)))
					overlaps.push_back(itGroup->first);
			}
		}

		if (overlaps.empty())
		{
			BushGroup* group = new BushGroup(this);
			m_bushGroupMap[group->id()] = group;
			group->addBush(bush);
			resideBushGroup(group);
			return group;
		}
		else
		{
			BushGroup* group = m_bushGroupMap[overlaps[0]];
			assert(group->zone());
			group->zone()->removeResideBushGroup(overlaps[0]);
			group->addBush(bush);

			for (size_t i = 1; i < overlaps.size(); ++i)
			{
				BushGroup* spliceGroup = m_bushGroupMap[overlaps[i]];
				assert(spliceGroup);
				group->splice(*spliceGroup);

				assert(spliceGroup->zone());
				spliceGroup->zone()->removeResideBushGroup(overlaps[i]);

				m_bushGroupMap.erase(overlaps[i]);
				delete spliceGroup;
			}

			resideBushGroup(group);
			return group;
		}
	}

	void Land::resideBushGroup(BushGroup* group)
	{
		assert(group->zone() == NULL);
		Zone* zone = dynamic_cast<Zone*>(locateTree(group->overall()));
		assert(zone);
		zone->addResideBushGroup(group);
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
				group->zone()->removeResideBushGroup(group->id());
			delete group;
		}
		m_bushGroupMap.clear();
	}

}
