#ifndef QTB_Land
#define QTB_Land

#include "Zone.h"
#include "Bush.h"

namespace qtb 
{
	class Land
		: public Zone
	{
	public:
		Land(const Area& area);
		virtual ~Land();

	public:
		unsigned int AllocBushID() {
			return m_nextBushID++;
		}

		unsigned int AllocBunchID() {
			return m_nextBunchID++;
		}

		void resetStaticBush(const AreaList& areaList);
		void generateBushMap(const AreaList& areaList, BushPMap& bushMap);

		const BushPMap& GetStaticBush() const { return m_staticBush; }

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBunchID;

	private:
		BushPMap		m_staticBush;
	};
}

#endif
