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
		Zone(const Area& area, Zone* parent = NULL);
		virtual QTree*	newChild(const Area& area);

	public:
		unsigned int			generation() const { return m_generation; }
		unsigned int			crossBushGroupID(float x, float y);

	public:
		void					addResideBushGroup(BushGroup* group);
		void					removeResideBushGroup(BushGroup* group);
		void					removeResideBushGroup(unsigned int groupID);
		const BushGroupPMap&	getResideBushGroup() const { return m_resideBushGroupMap; }

	private:
		bool					bushCross(float x, float y, unsigned int* bushID = NULL) const;

	private:
		unsigned int	m_generation;
		BushGroupPMap	m_resideBushGroupMap;
	};
}

#endif
