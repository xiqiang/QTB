#ifndef TreeBush_Land
#define TreeBush_Land

#include "Zone.h"
#include "Bush.h"

namespace qtb 
{
	class Land
		: public Zone
	{
	public:
		Land(const Area& area);

	public:
		unsigned int AllocBushID() {
			return m_nextBushID++;
		}

		unsigned int AllocBunchID() {
			return m_nextBunchID++;
		}

		void resetStaticBush(const AreaList& areaList);
		void generateBushMap(const AreaList& areaList, BushPMap& bushMap);

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBunchID;

	private:
		BushPMap		m_staticBush;
	};
}

#endif
