#ifndef TreeBunch_Bunch
#define TreeBunch_Bunch

#include <list>
#include "Area.h"

namespace treebunch 
{
	class Bunch
	{
	public:
		typedef std::list<Area> AreaList;

	private:
		AreaList m_areas;
	};
}

#endif
