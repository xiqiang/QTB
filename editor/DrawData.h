#ifndef Editor_DrawData
#define Editor_DrawData


#include <objidl.h>
#include <gdiplus.h>
#include <map>

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


private:
	BushResMap m_bushResMap;
	BushGroupResMap m_bushGroupResMap;
};

#endif
