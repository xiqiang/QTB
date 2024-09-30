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
		bool					rebuild(const AreaList& areas);
		unsigned int			createBush(const Area& area, Area* influence = NULL);
		bool					removeBush(unsigned int id, Area* influence = NULL);
		bool					bushTest(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL);

		const BushPMap&			bushes() const { return m_dynamicBushes; }
		const BushGroupPMap&	bushGroups() const { return m_bushGroups; }
		const BushPMap&			staticBushes() const { return m_staticBushes; }

	private:
		bool					generateBushMap(const AreaList& areas, BushPMap& bushes);

		BushGroup*				resideBush(Bush* bush);
		bool					resideBushGroup(BushGroup* group);
		bool					recycleBushGroup(BushGroup* group);

		void					clearBushMap(BushPMap& bushMap);
		void					clearBushGroupMap(BushGroupPMap& bushGroupMap);

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
