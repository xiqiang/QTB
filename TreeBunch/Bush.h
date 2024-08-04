#ifndef TreeBush_Bush
#define TreeBush_Bush

#include <map>
#include "Area.h"

namespace treebush 
{
	class Land;

	class Bush
	{
	public:
		Bush(Land* land, bool _static);

	public:
		unsigned int	id() const { return m_id; }
		bool			isStatic() const { return m_static; }
		const Area&		overall() const { return m_overall; }

		void			addArea(const Area& area);
		void			merge(const Bush& r);
		bool			overlap(const Bush& r) const;
		bool			overlap(const Area& area) const;
		bool			cross(float x, float y) const;

	private:
		Land*			m_land;
		unsigned int	m_id;
		bool			m_static;

	private:
		AreaList		m_areaList;
		Area			m_overall;
	};

	typedef std::list<Bush*>				BushPList;
	typedef std::map<unsigned int, Bush*>	BushPMap;

}

#endif
