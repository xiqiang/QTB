#ifndef Editor_DrawData
#define Editor_DrawData


#include <objidl.h>
#include <gdiplus.h>
#include <map>
#include "../base/Zone.h"

using namespace Gdiplus;

class DrawData
{
public:

	struct BushRes	
	{
		Color color;
	};
	typedef std::map<unsigned int, BushRes> BushResMap;

	struct BushGroupRes
	{
		Color color;
	};
	typedef std::map<unsigned int, BushGroupRes> BushGroupResMap;

	struct ZoneRes
	{
		Color color;
	};
	typedef std::map<qtb::Zone*, ZoneRes> ZoneResMap;

	struct ZoneGenerationRes
	{
		Color color;
	};
	typedef std::map<unsigned int, ZoneGenerationRes> ZoneGenerationResMap;

public:
	const BushRes& GetBushRes(unsigned int id) {
		BushResMap::iterator it = m_bushResMap.find(id);
		if (it != m_bushResMap.end())
			return it->second;

		BushRes res;
		res.color = Color(192, rand() % 256, rand() % 256, rand() % 256);

		std::pair<BushResMap::iterator, bool> ret = m_bushResMap.insert( std::pair<unsigned int, BushRes>(id, res));
		assert(ret.second);
		return ret.first->second;
	}

	const BushGroupRes& GetBushGroupRes(unsigned int id) {
		BushGroupResMap::iterator it = m_bushGroupResMap.find(id);
		if (it != m_bushGroupResMap.end())
			return it->second;

		BushGroupRes res;
		res.color = Color(255, rand() % 256, rand() % 256, rand() % 256);

		std::pair<BushGroupResMap::iterator, bool> ret = m_bushGroupResMap.insert(std::pair<unsigned int, BushGroupRes>(id, res));
		assert(ret.second);
		return ret.first->second;
	}

	const ZoneRes& GetZoneRes(qtb::Zone* zone) {
		ZoneResMap::iterator it = m_zoneResMap.find(zone);
		if (it != m_zoneResMap.end())
			return it->second;

		ZoneRes res;
		res.color = Color(255, rand() % 256, rand() % 256, rand() % 256);

		std::pair<ZoneResMap::iterator, bool> ret = m_zoneResMap.insert(std::pair<qtb::Zone*, ZoneRes>(zone, res));
		assert(ret.second);
		return ret.first->second;
	}

	const ZoneGenerationRes& GetZoneGenerationRes(unsigned int id) {
		ZoneGenerationResMap::iterator it = m_zoneGenerationResMap.find(id);
		if (it != m_zoneGenerationResMap.end())
			return it->second;

		ZoneGenerationRes res;
		res.color = Color(255, rand() % 256, rand() % 256, rand() % 256);

		std::pair<ZoneGenerationResMap::iterator, bool> ret = m_zoneGenerationResMap.insert(std::pair<unsigned int, ZoneGenerationRes>(id, res));
		assert(ret.second);
		return ret.first->second;
	}


private:
	BushResMap				m_bushResMap;
	BushGroupResMap			m_bushGroupResMap;
	ZoneResMap				m_zoneResMap;
	ZoneGenerationResMap	m_zoneGenerationResMap;
};

#endif
