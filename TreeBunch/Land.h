#ifndef TreeBush_Land
#define TreeBush_Land

#include "Zone.h"
#include "Bush.h"

namespace treebush 
{
	class Land
		: public Zone
	{
	public:
		Land(const Area& area, float minZoneSize)
			: Zone(area, minZoneSize, NULL)
			, m_nextBushID(0)
			, m_nextBunchID(0)
		{}

	public:
		unsigned int AllocBushID() {
			return m_nextBushID++;
		}

		unsigned int AllocBunchID() {
			return m_nextBunchID++;
		}

		void resetStaticBush(const AreaVector& areaList) {
			BushPMap bushMap;

			for (AreaList::const_iterator it = areaList.begin(); it != areaList.end(); ++it)
			{

			}
		}

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBunchID;

	private:
		BushPList		m_staticBushList;
	};
}

#endif
