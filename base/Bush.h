#ifndef QTB_Bush
#define QTB_Bush

#include <map>
#include "Area.h"

namespace qtb 
{
	class BushGroup;

	class Bush
	{
	public:
		friend class Land;
		friend class BushGroup;

	private:
		Bush(unsigned int id);

	public:
		unsigned int	id() const { return m_id; }
		BushGroup*		group() const { return m_group; }
		const Area&		overall() const { return m_overall; }

		bool			overlap(const Bush& other) const;
		bool			overlap(const Area& area) const;
		bool			contains(float x, float y) const;

		void			add(const Area& area);
		void			splice(Bush& other);

	private:
		unsigned int	m_id;
		BushGroup*		m_group;

	private:
		AreaList		m_areaList;
		Area			m_overall;
	};

	typedef std::list<Bush*>				BushPList;
	typedef std::map<unsigned int, Bush*>	BushPMap;

}

#endif
