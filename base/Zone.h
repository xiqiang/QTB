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
		unsigned int	generation() const { return m_generation; }

	public:
		bool			bushCross(float x, float y, unsigned int* bushID = NULL) const;
		void			addResideBushGroup(BushGroup* group);
		void			removeResideBushGroup(BushGroup* group);

	private:
		unsigned int	m_generation;
		BushGroupPMap	m_resideBushGroupMap;
	};
}

#endif
