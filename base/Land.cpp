#include <cassert>
#include <vector>
#include "Land.h"

namespace qtb
{
	Land::Land(const Area& area)
		: Zone(area, NULL)
		, m_nextBushID(0)
		, m_nextBushGroupID(0)
	{
	}

	Land::~Land()
	{
		clear();
	}

	void Land::clear()
	{
		clearBushMap(m_staticBushes);
		clearBushMap(m_dynamicBushes);
		clearBushGroup();
	}

	void Land::rebuild(const AreaList& areas)
	{
		clear();

		generateBushMap(areas, m_staticBushes);
		for (BushPMap::iterator itBush = m_staticBushes.begin(); itBush != m_staticBushes.end(); ++itBush)
		{
			BushGroup* group = new BushGroup(m_nextBushGroupID++);
			m_bushGroups[group->id()] = group;
			group->add(itBush->second);

			resideBushGroup(group);
		}
	}

	unsigned int Land::createBush(const Area& area, Area* influence /*= NULL*/)
	{
		Bush* bush = new Bush(m_nextBushID++);
		bush->add(area);
		m_dynamicBushes[bush->id()] = bush;

		BushGroup* group = resideBush(bush);
		assert(group);

		if (influence)
			*influence = group->overall();

		return bush->id();
	}

	bool Land::removeBush(unsigned int id, Area* influence /*= NULL*/)
	{
		BushPMap::iterator itFind = m_dynamicBushes.find(id);
		if (m_dynamicBushes.end() == itFind)
			return false;

		Bush* bush = itFind->second;
		assert(bush);
		BushGroup* group = bush->group();
		assert(group);

		if (influence)
			*influence = group->overall();

		group->remove(id);
		recycleBushGroup(group);

		m_dynamicBushes.erase(id);
		delete bush;

		return true;
	}

	void Land::generateBushMap(const AreaList& areaMap, BushPMap& bushMap)
	{
		bushMap.clear();

		for (AreaList::const_iterator itArea = areaMap.begin(); itArea != areaMap.end(); ++itArea)
		{
			std::vector<unsigned> collisions;
			for (BushPMap::iterator itBush = bushMap.begin(); itBush != bushMap.end(); ++itBush)
			{
				if (itBush->second->overlap(*itArea))
					collisions.push_back(itBush->first);
			}

			if (collisions.empty())
			{
				Bush* bush = new Bush(m_nextBushID++);
				bushMap[bush->id()] = bush;
				bush->add(*itArea);
			}
			else
			{
				Bush* bush = bushMap[collisions[0]];
				bush->add(*itArea);
				for (size_t i = 1; i < collisions.size(); ++i)
				{
					Bush* spliceBush = bushMap[collisions[i]];
					assert(spliceBush);
					bush->splice(*spliceBush);

					bushMap.erase(collisions[i]);
					delete spliceBush;
				}
			}
		}
	}

	BushGroup* Land::resideBush(Bush* bush)
	{
		assert(bush);

		std::vector<unsigned> collisions;
		std::list<QTree*> treeList;
		layer(bush->overall(), treeList);

		for (std::list<QTree*>::iterator itTree = treeList.begin(); itTree != treeList.end(); ++itTree)
		{
			Zone* zone = dynamic_cast<Zone*>(*itTree);
			assert(zone);

			const BushGroupPMap& resideBushGroupMap = zone->bushGroups();
			for (BushGroupPMap::const_iterator itGroup = resideBushGroupMap.begin(); itGroup != resideBushGroupMap.end(); ++itGroup)
			{
				if (itGroup->second->overlap(*(bush)))
					collisions.push_back(itGroup->first);
			}
		}

		if (collisions.empty())
		{
			BushGroup* group = new BushGroup(m_nextBushGroupID++);
			m_bushGroups[group->id()] = group;
			group->add(bush);
			resideBushGroup(group);
			return group;
		}
		else
		{
			BushGroup* group = m_bushGroups[collisions[0]];
			assert(group->zone());
			group->zone()->removeBushGroup(collisions[0]);
			group->add(bush);

			for (size_t i = 1; i < collisions.size(); ++i)
			{
				BushGroup* spliceGroup = m_bushGroups[collisions[i]];
				assert(spliceGroup);
				group->splice(*spliceGroup);

				assert(spliceGroup->zone());
				spliceGroup->zone()->removeBushGroup(collisions[i]);

				m_bushGroups.erase(collisions[i]);
				delete spliceGroup;
			}

			resideBushGroup(group);
			return group;
		}
	}

	void Land::resideBushGroup(BushGroup* group)
	{
		assert(group->zone() == NULL);
		Zone* zone = dynamic_cast<Zone*>(locate(group->overall()));
		assert(zone);
		zone->addBushGroup(group);
	}

	void Land::recycleBushGroup(BushGroup* group)
	{
		assert(group);
		assert(m_bushGroups.find(group->id()) != m_bushGroups.end());

		assert(group->zone());
		group->zone()->removeBushGroup(group->id());

		BushGroupPMap groupMap;
		const BushPMap& bushMap = group->bushes();

		for (BushPMap::const_iterator itBush = bushMap.begin(); itBush != bushMap.end(); ++itBush)
		{
			std::vector<unsigned> collisions;
			for (BushGroupPMap::iterator itGroup = groupMap.begin(); itGroup != groupMap.end(); ++itGroup)
			{
				if (itGroup->second->overlap(*(itBush->second)))
					collisions.push_back(itGroup->first);
			}

			if (collisions.empty())
			{
				BushGroup* cacheGroup = new BushGroup(m_nextBushGroupID++);
				groupMap[cacheGroup->id()] = cacheGroup;
				cacheGroup->add(itBush->second);
			}
			else
			{
				BushGroup* cacheGroup = groupMap[collisions[0]];
				cacheGroup->add(itBush->second);
				for (size_t i = 1; i < collisions.size(); ++i)
				{
					BushGroup* spliceGroup = groupMap[collisions[i]];
					assert(spliceGroup);
					cacheGroup->splice(*spliceGroup);

					groupMap.erase(collisions[i]);
					delete spliceGroup;
				}
			}
		}

		for (BushGroupPMap::const_iterator itGroup = groupMap.begin(); itGroup != groupMap.end(); ++itGroup)
			resideBushGroup(itGroup->second);

		m_bushGroups.insert(groupMap.begin(), groupMap.end());
		m_bushGroups.erase(group->id());
		delete group;
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
		for (BushGroupPMap::iterator it = m_bushGroups.begin(); it != m_bushGroups.end(); ++it)
		{
			BushGroup* group = it->second;
			assert(group);
			if (group->zone())
				group->zone()->removeBushGroup(group->id());
			delete group;
		}
		m_bushGroups.clear();
	}

}
