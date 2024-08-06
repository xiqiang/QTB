#ifndef QTB_Bush
#define QTB_Bush

#include <map>
#include "Area.h"

namespace qtb 
{
	class Land;

	class Bush
	{
	public:
		Bush(Land* land);

	public:
		unsigned int	id() const { return m_id; }
		const Area&		overall() const { return m_overall; }

		void			addArea(const Area& area);
		void			splice(Bush& r);
		bool			overlap(const Bush& r) const;
		bool			overlap(const Area& area) const;
		bool			cross(float x, float y) const;

	private:
		Land*			m_land;
		unsigned int	m_id;

	private:
		AreaList		m_areaList;
		Area			m_overall;
	};

	typedef std::list<Bush*>				BushPList;
	typedef std::map<unsigned int, Bush*>	BushPMap;

}

#endif
