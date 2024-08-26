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
		unsigned int		id() const { return m_id; }
		const Area&			overall() const { return m_overall; }
		bool				isStatic() const { return m_isStatic; }

		const BushGroup*	group() const { return m_group; }
		const AreaList&		areas() const { return m_areaList; }

		bool				overlap(const Bush& other) const;
		bool				overlap(const Area& area) const;
		bool				contains(float x, float y) const;

	private:
		void				add(const Area& area);
		void				splice(Bush& other);

	private:
		unsigned int		m_id;
		BushGroup*			m_group;
		AreaList			m_areaList;
		Area				m_overall;
		bool				m_isStatic;
	};

	typedef std::list<Bush*>				BushPList;
	typedef std::map<unsigned int, Bush*>	BushPMap;

}

#endif
