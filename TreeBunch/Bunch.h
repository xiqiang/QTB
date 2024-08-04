#ifndef TreeBunch_Bunch
#define TreeBunch_Bunch

#include <cassert>
#include <list>
#include "Bush.h"

namespace treebush 
{
	class Land;
	class Zone;

	class Bunch
	{
	public:
		friend class Zone;

	public:
		typedef std::list<Bush*> BushList;

	public:
		Bunch(Land* land) {
			assert(land);
			m_land = land;
			m_id = land->AllocBunchID();
			m_zone = NULL;
		}

	public:
		unsigned int id() const { return m_id; }
		const Area& entireArea() { return m_entireArea; }

		void addBush(Bush* bush) {
			assert(bush);
			const Area& bushArea = bush->entireArea();
			m_entireArea.left	= m_entireArea.left		< bushArea.left		? m_entireArea.left		: bushArea.left;
			m_entireArea.right	= m_entireArea.right	> bushArea.right	? m_entireArea.right	: bushArea.right;
			m_entireArea.bottom	= m_entireArea.bottom	< bushArea.bottom	? m_entireArea.bottom	: bushArea.bottom;
			m_entireArea.top	= m_entireArea.top		> bushArea.top		? m_entireArea.top		: bushArea.top;
			m_bushList.push_back(bush);
		}

		bool bushCheck(float x, float y) const {
			if (!m_entireArea.contains(x, y))
				return false;

			for (BushList::const_iterator it = m_bushList.begin(); it != m_bushList.end(); ++it) {
				const Bush* bush = *it;
				assert(bush);
				if (bush->check(x, y))
					return true;
			}
			return false;
		}

	private:
		Land*			m_land;
		Zone*			m_zone;
		unsigned int	m_id;

	private:
		BushList	m_bushList;
		Area		m_entireArea;
	};
}

#endif
