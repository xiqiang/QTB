#ifndef TreeLand_Land
#define TreeLand_Land

#include <list>
#include "Zone.h"

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

	private:
		unsigned int m_nextBushID;
		unsigned int m_nextBunchID;
	};
}

#endif
