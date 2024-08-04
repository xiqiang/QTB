#ifndef TreeBush_Zone
#define TreeBush_Zone

#include "QTree.h"
#include "Bunch.h"

namespace treebush 
{
	class Zone
		: public QTree<Zone>
	{
	public:
		Zone(const Area& area, float minZoneSize, Zone* parent = NULL);

	public:
		void		addBunch(Bunch* bunch);
		bool		bushCross(float x, float y, unsigned int* bushID = NULL) const;

	private:
		BunchMap	m_bunchMap;
	};
}

#endif
