#ifndef Editor_DrawData
#define Editor_DrawData


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

public:
	const BushRes& GetBushRes(unsigned int id) {
		BushResMap::iterator it = m_bushBushResMap.find(id);
		if (it != m_bushBushResMap.end())
			return it->second;

		BushRes res;
		res.color = Color(192, rand() % 256, rand() % 256, rand() % 256);

		std::pair<BushResMap::iterator, bool> ret = m_bushBushResMap.insert( std::pair<unsigned int, BushRes>(id, res));
		assert(ret.second);
		return ret.first->second;
	}

private:
	BushResMap m_bushBushResMap;
};

#endif
