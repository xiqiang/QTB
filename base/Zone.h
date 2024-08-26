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

	protected:
		virtual QTree*			newChild(const Area& area);

	private:
		void					bindBushGroup(BushGroup* group);
		void					unbindBushGroup(unsigned int groupID);

	private:
		bool					_bushContains(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL) const;

	private:
		BushGroupPMap			m_boundBushGroups;
	};
}

#endif
