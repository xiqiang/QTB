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
		void			addBushGroup(BushGroup* bunch);
		bool			bushCross(float x, float y, unsigned int* bushID = NULL) const;

	private:
		unsigned int	m_generation;
		BushGroupPMap	m_bushGroupMap;
	};
}

#endif
