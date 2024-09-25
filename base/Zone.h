#ifndef QTB_Zone
#define QTB_Zone

#include "QTree.h"
#include "BushGroup.h"

namespace qtb 
{
	class Zone
		: public QTree
	{
	public:
		friend class Land;

	private:
		Zone(const Area& area, Zone* parent = NULL);

	public:
		bool					bushContains(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL);
		const BushGroupPMap&	boundBushGroups() const { return m_boundBushGroups; }
		size_t					fullBindCount() const { return m_boundBushGroups.size() + m_childBindCount; }

	protected:
		virtual QTree*			newChild(const Area& area);
		virtual bool			layerTest() const { return fullBindCount() > 0; }

	private:
		void					bindBushGroup(BushGroup* group);
		void					unbindBushGroup(unsigned int groupID);

	private:
		bool					_bushContains(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL) const;
		void					_incChildBindCount();
		void					_decChildBindCount();

	private:
		BushGroupPMap			m_boundBushGroups;
		size_t					m_childBindCount;
	};
}

#endif
