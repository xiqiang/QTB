#ifndef TreeBush_Zone
#define TreeBush_Zone

#include "QTree.h"
#include "Bunch.h"

namespace qtb 
{
	class Zone
		: public QTree
	{
	public:
		Zone(const Area& area, Zone* parent = NULL);

		virtual QTree*	newChild(const Area& area);

	public:
		void		addBunch(Bunch* bunch);
		bool		bushCross(float x, float y, unsigned int* bushID = NULL) const;

	private:
		BunchMap	m_bunchMap;
	};
}

#endif
