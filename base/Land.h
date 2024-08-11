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
		void					clear();

		void					rebuild(const AreaList& areas);
		const BushPMap&			staticBushes() const { return m_staticBushes; }

		unsigned int			createBush(const Area& area, Area* influence = NULL);
		bool					removeBush(unsigned int id, Area* influence = NULL);
		const BushPMap&			bushes() const { return m_dynamicBushes; }

		const BushGroupPMap&	bushGroups() const { return m_bushGroups; }

	private:
		void					generateBushMap(const AreaList& areas, BushPMap& bushes);

		BushGroup*				resideBush(Bush* bush);
		void					resideBushGroup(BushGroup* group);
		void					recycleBushGroup(BushGroup* group);

		void					clearBushMap(BushPMap& bushMap);
		void					clearBushGroup();

	private:
		unsigned int	m_nextBushID;
		unsigned int	m_nextBushGroupID;

	private:
		BushPMap		m_staticBushes;
		BushPMap		m_dynamicBushes;
		BushGroupPMap	m_bushGroups;

	};
}

#endif
