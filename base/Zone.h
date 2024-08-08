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
		bool					bushCross(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL);

	public:
		void					addResideBushGroup(BushGroup* group);
		void					removeResideBushGroup(unsigned int groupID);
		const BushGroupPMap&	getResideBushGroup() const { return m_resideBushGroupMap; }

	private:
		bool					_bushCross(float x, float y, unsigned int* bushGroupID = NULL, unsigned int* bushID = NULL) const;

	private:
		unsigned int	m_generation;
		BushGroupPMap	m_resideBushGroupMap;
	};
}

#endif
