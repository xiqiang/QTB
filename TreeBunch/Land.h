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
		Land(const Area& area, float minZoneSize);

	public:
		unsigned int AllocBushID() {
			return m_nextBushID++;
		}

		unsigned int AllocBunchID() {
			return m_nextBunchID++;
		}

		void resetStaticBush(const AreaList& areaList);

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBunchID;

	private:
		BushPList		m_staticBushList;
	};
}

#endif
