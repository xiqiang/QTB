#include <cassert>
#include <vector>
#include "Land.h"
#include "Util.h"

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
		clearBushGroupMap(m_bushGroups);
		clearBushMap(m_dynamicBushes);
		clearBushMap(m_staticBushes);
	}

	bool Land::rebuild(const AreaList& areas)
	{
		clear();

		generateBushMap(areas, m_staticBushes);
		if (m_staticBushes.empty())
			return false;

		BushPMap::const_iterator itBushEnd = m_staticBushes.end();
		for (BushPMap::const_iterator itBush = m_staticBushes.begin(); itBush != itBushEnd; ++itBush)
		{
			BushGroup* group = NULL;
			try
			{
				QTB_RAND_BAD_ALLOC(1);
				group = new BushGroup(m_nextBushGroupID++);
				if (!group)
					return false;

				QTB_RAND_BAD_ALLOC(1);
				std::pair<BushGroupPMap::iterator, bool> ret = m_bushGroups.insert(
					std::pair<unsigned int, BushGroup*>(group->id(), group));
			}
			catch (std::bad_alloc&)
			{
				if (group)
					delete group;

				qtbLog("bad_alloc:  Land::rebuild\n");
				return false;
			}

			if (!group->add(itBush->second))
				return false;

			if (!resideBushGroup(group))
				return false;
		}

		return true;
	}

	unsigned int Land::createBush(const Area& area, Area* influence /*= NULL*/)
	{
		Bush* bush = NULL;
		try
		{
			QTB_RAND_BAD_ALLOC(1);
			bush = new Bush(m_nextBushID++);
			if(!bush)
				return UINT_MAX;

			QTB_RAND_BAD_ALLOC(1);
			std::pair<BushPMap::iterator, bool> ret = m_dynamicBushes.insert(
				std::pair<unsigned int, Bush*>(bush->id(), bush));
		}
		catch (std::bad_alloc&)
		{
			if (bush)
				delete bush;

			qtbLog("bad_alloc:  Land::createBush\n");
			return UINT_MAX;
		}

		if(!bush->add(area))
			return bush->id();

		BushGroup* group = resideBush(bush);
		if (!group)
			return bush->id();

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

		BushGroup* group = bush->m_group;
		if (group)
		{
			if (influence)
				*influence = group->overall();
			group->remove(id);
		}

		m_dynamicBushes.erase(id);
		delete bush;

		bool ret = true;
		if (group)
			ret = recycleBushGroup(group);

		return ret;
	}

	bool Land::bushTest(float x, float y, unsigned int* bushGroupID /*= NULL*/, unsigned int* bushID /*= NULL*/)
	{
		Zone* zone = dynamic_cast<Zone*>(locate(x, y));
		assert(zone);
		return zone->bushContainsRev(x, y, bushGroupID, bushID);
		//return bushContains(x, y, bushGroupID, bushID);
	}

	bool Land::generateBushMap(const AreaList& areaMap, BushPMap& bushMap)
	{
		assert(bushMap.empty());

		AreaList::const_iterator itAreaEnd = areaMap.end();
		for (AreaList::const_iterator itArea = areaMap.begin(); itArea != itAreaEnd; ++itArea)
		{
			std::vector<unsigned> collisions;
			if (!bushMap.empty())
			{
				BushPMap::const_iterator itBushEnd = bushMap.end();
				for (BushPMap::const_iterator itBush = bushMap.begin(); itBush != itBushEnd; ++itBush)
				{
					if (itBush->second->overlap(*itArea))
					{
						try
						{
							QTB_RAND_BAD_ALLOC(1);
							collisions.push_back(itBush->first);
						}
						catch (std::bad_alloc&)
						{
							qtbLog("bad_alloc:  Land::generateBushMap collisions\n");
							return false;
						}
					}
				}
			}

			if (collisions.empty())
			{
				Bush* bush = NULL;
				try
				{
					QTB_RAND_BAD_ALLOC(1);
					bush = new Bush(m_nextBushID++);
					if (!bush)
						return false;

					QTB_RAND_BAD_ALLOC(1);
					std::pair<BushPMap::iterator, bool> ret = bushMap.insert(
						std::pair<unsigned int, Bush*>(bush->id(), bush));
				}
				catch (std::bad_alloc&)
				{
					if (bush)
						delete bush;

					qtbLog("bad_alloc:  Land::generateBushMap bush\n");
					return false;
				}

				bush->m_isStatic = true;
				if (!bush->add(*itArea))
					return false;
			}
			else
			{
				Bush* bush = bushMap[collisions[0]];
				if (!bush->add(*itArea))
					return false;

				for (size_t i = 1; i < collisions.size(); ++i)
				{
					Bush* spliceBush = bushMap[collisions[i]];
					assert(spliceBush);
					if (!bush->splice(*spliceBush))
						return false;

					bushMap.erase(collisions[i]);
					delete spliceBush;
				}
			}
		}

		return true;
	}

	BushGroup* Land::resideBush(Bush* bush)
	{
		assert(bush);

		std::list<QTree*> treeList;
		if (!layer(bush->overall(), treeList) || treeList.empty())
			return NULL;

		std::vector<unsigned> collisions;
		std::list<QTree*>::const_iterator itTreeEnd = treeList.end();
		for (std::list<QTree*>::const_iterator itTree = treeList.begin(); itTree != itTreeEnd; ++itTree)
		{
			Zone* zone = dynamic_cast<Zone*>(*itTree);
			assert(zone);

			const BushGroupPMap& resideBushGroupMap = zone->boundBushGroups();
			BushGroupPMap::const_iterator itGroupEnd = resideBushGroupMap.end();
			for (BushGroupPMap::const_iterator itGroup = resideBushGroupMap.begin(); itGroup != itGroupEnd; ++itGroup)
			{
				if (itGroup->second->overlap(*(bush)))
				{
					try
					{
						QTB_RAND_BAD_ALLOC(1);
						collisions.push_back(itGroup->first);
					}
					catch (std::bad_alloc&)
					{
						qtbLog("bad_alloc:  Land::resideBush collisions\n");
						return NULL;
					}
				}
			}
		}

		if (collisions.empty())
		{
			BushGroup* group = NULL;
			try
			{
				QTB_RAND_BAD_ALLOC(1);
				group = new BushGroup(m_nextBushGroupID++);
				if (!group)
					return NULL;

				QTB_RAND_BAD_ALLOC(1);
				std::pair<BushGroupPMap::iterator, bool> ret = m_bushGroups.insert(
					std::pair<unsigned int, BushGroup*>(group->id(), group));
			}
			catch (std::bad_alloc&)
			{
				if (group)
					delete group;

				qtbLog("bad_alloc:  Land::resideBush bushGroup\n");
				return NULL;
			}

			if (!group->add(bush))
				return NULL;

			if (!resideBushGroup(group))
				return NULL;

			return group;
		}

		BushGroup* group = m_bushGroups[collisions[0]];
		if (!group->add(bush))
			return NULL;

		assert(group->m_zone);
		group->m_zone->unbindBushGroup(collisions[0]);

		for (size_t i = 1; i < collisions.size(); ++i)
		{
			BushGroup* spliceGroup = m_bushGroups[collisions[i]];
			assert(spliceGroup);
			if (!group->splice(*spliceGroup))
				return NULL;

			assert(spliceGroup->m_zone);
			spliceGroup->m_zone->unbindBushGroup(collisions[i]);

			m_bushGroups.erase(collisions[i]);
			delete spliceGroup;
		}

		if (!resideBushGroup(group))
			return NULL;

		return group;
	}

	bool Land::resideBushGroup(BushGroup* group)
	{
		assert(group->zone() == NULL);

		Zone* zone = dynamic_cast<Zone*>(deepLocate(group->overall()));
		assert(zone);
		return zone->bindBushGroup(group);
	}

	bool Land::recycleBushGroup(BushGroup* group)
	{
		assert(group);
		assert(m_bushGroups.find(group->id()) != m_bushGroups.end());

		group->releaseBushes();
		if (group->m_zone)
			group->m_zone->unbindBushGroup(group->id());

		BushPMap bushMap;
		bushMap.swap(group->m_bushes);
		m_bushGroups.erase(group->id());
		delete group;

		BushGroupPMap groupMap;
		BushPMap::const_iterator itBushEnd = bushMap.end();
		for (BushPMap::const_iterator itBush = bushMap.begin(); itBush != itBushEnd; ++itBush)
		{
			std::vector<unsigned> collisions;
			if (!groupMap.empty())
			{
				BushGroupPMap::iterator itGroupEnd = groupMap.end();
				for (BushGroupPMap::iterator itGroup = groupMap.begin(); itGroup != itGroupEnd; ++itGroup)
				{
					if (itGroup->second->overlap(*(itBush->second)))
					{
						try
						{
							QTB_RAND_BAD_ALLOC(1);
							collisions.push_back(itGroup->first);
						}
						catch (std::bad_alloc&)
						{
							clearBushGroupMap(groupMap);
							qtbLog("bad_alloc:  Land::recycleBushGroup collisions\n");
							return false;
						}
					}
				}
			}

			if (collisions.empty())
			{
				BushGroup* cacheGroup = NULL;
				try
				{
					QTB_RAND_BAD_ALLOC(1);
					cacheGroup = new BushGroup(m_nextBushGroupID++);
					if (!cacheGroup)
					{
						clearBushGroupMap(groupMap);
						return false;
					}

					QTB_RAND_BAD_ALLOC(1);
					std::pair<BushGroupPMap::iterator, bool> ret = groupMap.insert(
						std::pair<unsigned int, BushGroup*>(cacheGroup->id(), cacheGroup));
				}
				catch (std::bad_alloc&)
				{
					if (cacheGroup)
						delete cacheGroup;

					clearBushGroupMap(groupMap);
					qtbLog("bad_alloc:  Land::recycleBushGroup groupMap\n");
					return false;
				}

				if (!cacheGroup->add(itBush->second))
				{
					clearBushGroupMap(groupMap);
					return false;
				}
			}
			else
			{
				BushGroup* cacheGroup = groupMap[collisions[0]];
				if (!cacheGroup->add(itBush->second))
				{
					clearBushGroupMap(groupMap);
					return false;
				}

				for (size_t i = 1; i < collisions.size(); ++i)
				{
					BushGroup* spliceGroup = groupMap[collisions[i]];
					assert(spliceGroup);
					if (!cacheGroup->splice(*spliceGroup))
					{
						clearBushGroupMap(groupMap);
						return false;
					}

					groupMap.erase(collisions[i]);
					delete spliceGroup;
				}
			}
		}

		BushGroupPMap::const_iterator itGroupBegin = groupMap.begin();
		BushGroupPMap::const_iterator itGroupEnd = groupMap.end();

		try
		{
			QTB_RAND_BAD_ALLOC(1);
			m_bushGroups.insert(itGroupBegin, itGroupEnd);
		}
		catch (std::bad_alloc&)
		{
			clearBushGroupMap(groupMap);
			qtbLog("bad_alloc:  Land::recycleBushGroup m_bushGroups\n");
			return false;
		}

		for (BushGroupPMap::const_iterator itGroup = itGroupBegin; itGroup != itGroupEnd; ++itGroup)
		{
			if (!resideBushGroup(itGroup->second))
				return false;
		}

		return true;
	}

	void Land::clearBushMap(BushPMap& bushMap)
	{
		for (BushPMap::iterator it = bushMap.begin(); it != bushMap.end(); ++it)
		{
			Bush* bush = it->second;
			assert(bush);
			delete bush;
		}
		bushMap.clear();
	}

	void Land::clearBushGroupMap(BushGroupPMap& bushGroupMap)
	{
		for (BushGroupPMap::iterator it = bushGroupMap.begin(); it != bushGroupMap.end(); ++it)
		{
			BushGroup* group = it->second;
			assert(group);
			group->releaseBushes();

			if (group->m_zone)
				group->m_zone->unbindBushGroup(group->id());

			delete group;
		}
		bushGroupMap.clear();
	}

}
