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

		unsigned int AllocBushGroupID() {
			return m_nextBushGroupID++;
		}

		unsigned int AllocAreaID() {
			return m_nextAreaID++;
		}

		void					resetStaticBush(const AreaMap& areaMap);
		const BushPMap&			getStaticBush() const { return m_staticBushMap; }

		unsigned int			addDynamicArea(const Area& area);
		void					removeDynamicArea(unsigned int id);
		void					setDynamicAreas(const AreaMap& areas);

		void					generateBushMap(const AreaMap& areaMap, BushPMap& bushMap);
		void					clearBushMap(BushPMap& bushMap);

		void					rebuildBushGroup();
		void					clearBushGroup();
		const BushGroupPMap&	getBushGroup() const { return m_bushGroupMap; }

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBushGroupID;
		unsigned int	m_nextAreaID;

	private:
		BushPMap		m_staticBushMap;
		AreaMap			m_dynamicAreaMap;
		BushGroupPMap	m_bushGroupMap;

	};
}

#endif
