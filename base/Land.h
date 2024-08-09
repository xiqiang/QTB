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

		void					rebuild(const AreaMap& areaMap);
		unsigned int			createDynamicBush(const Area& area, Area* influence = NULL);
		bool					removeDynamicBush(unsigned int id, Area* influence = NULL);

		const BushPMap&			getStaticBush() const { return m_staticBushMap; }
		const BushPMap&			getDynamicBush() const { return m_dynamicBushMap; }
		const BushGroupPMap&	getBushGroup() const { return m_bushGroupMap; }

	private:
		void					generateBushMap(const AreaMap& areaMap, BushPMap& bushMap);
		void					recycleBushGroup(BushGroup* group);

		BushGroup*				resideBush(Bush* bush);
		void					resideBushGroup(BushGroup* group);

		void					clearBushMap(BushPMap& bushMap);
		void					clearBushGroup();

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBushGroupID;
		unsigned int	m_nextAreaID;

	private:
		BushPMap		m_staticBushMap;
		BushPMap		m_dynamicBushMap;
		BushGroupPMap	m_bushGroupMap;

	};
}

#endif
