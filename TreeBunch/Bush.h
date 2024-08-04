#ifndef TreeBush_Bush
#define TreeBush_Bush

#include <list>
#include "Area.h"

namespace treebush 
{
	class Land;

	class Bush
	{
	public:
		typedef std::list<Area> AreaList;

	public:
		Bush(Land* land) {
			assert(land);
			m_land = land;
			m_id = land->AllocBushID();
		}

	public:
		unsigned int id() const { return m_id; }
		const Area& entireArea() const { return m_entireArea; }

		void addArea(const Area& area) {
			m_entireArea.left	= m_entireArea.left		< area.left		? m_entireArea.left		: area.left;
			m_entireArea.right	= m_entireArea.right	> area.right	? m_entireArea.right	: area.right;
			m_entireArea.bottom	= m_entireArea.bottom	< area.bottom	? m_entireArea.bottom	: area.bottom;
			m_entireArea.top	= m_entireArea.top		> area.top		? m_entireArea.top		: area.top;
			m_areaList.push_back(area);
		}

		bool isOverlap(const Bush& r) const {
			if (!m_entireArea.isOverlap(r.m_entireArea))
				return false;

			size_t size = m_areaList.size();
			size_t size_r = r.m_areaList.size();

			for (AreaList::const_iterator it = m_areaList.begin(); it != m_areaList.end(); ++it) {
				if (size > 1 && size_r > 1 && !it->isOverlap(r.m_entireArea))
					continue;

				for (AreaList::const_iterator it_r = r.m_areaList.begin(); it_r != r.m_areaList.end(); ++it_r) {
					if (it->isOverlap(*it_r))
						return true;
				}
			}
			return false;
		}

		bool check(float x, float y) const {
			if (!m_entireArea.contains(x, y))
				return false;

			for (AreaList::const_iterator it = m_areaList.begin(); it != m_areaList.end(); ++it) {
				if (it->contains(x, y))
					return true;
			}
			return false;
		}
	private:
		Land*			m_land;
		unsigned int	m_id;

	private:
		AreaList	m_areaList;
		Area		m_entireArea;
	};
}

#endif
