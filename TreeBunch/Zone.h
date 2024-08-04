#ifndef TreeZone_Zone
#define TreeZone_Zone

#include <cassert>
#include <map>
#include "Area.h"
#include "Bunch.h"

namespace treebush 
{
	class Zone
	{
	public:
		typedef std::map<unsigned int, Bunch*> BunchMap;

		static const int left_bottom	= 0;
		static const int left_top		= 1;
		static const int right_bottom	= 2;
		static const int right_top		= 3;

	public:
		Zone(const Area& area, float minZoneSize, Zone* parent = NULL) {
			m_area = area;
			m_parent = parent;
			memset(m_childs, 0, sizeof(m_childs));
			devide(minZoneSize);
		}

	public:
		const Area& area() { return m_area; }

		void addBunch(Bunch* bunch) {
			assert(bunch);
			assert(NULL == bunch->m_zone);

			bunch->m_zone = this;
			assert(m_bunchMap.find(bunch->id()) == m_bunchMap.end());
			m_bunchMap[bunch->id()] = bunch;
		}

		bool bushCheck(float x, float y, unsigned int* bushID = NULL) const {
			if (!m_area.contains(x, y))
				return false;

			for (BunchMap::const_iterator it = m_bunchMap.begin(); it != m_bunchMap.end(); ++it) {
				Bunch* bunch = it->second;
				assert(bunch);

				if (bunch->bushCheck(x, y)) {
					if (bushID)
						*bushID = bunch->id();
					return true;
				}
			}

			return false;
		}

		void devide(float minZoneSize) {
			float childWidth = m_area.width() * 0.5;
			float childHeight = m_area.height() * 0.5;

			if (childWidth <= minZoneSize || childHeight <= minZoneSize)
				return;

			float cx = m_area.x();
			float cy = m_area.y();

			Area areaLB(cx - childWidth, cx, cy - childHeight, cy);
			m_childs[left_bottom] = new Zone(areaLB, minZoneSize, this);

			Area areaLT(cx - childWidth, cx, cy, cy + childHeight);
			m_childs[left_top] = new Zone(areaLT, minZoneSize, this);

			Area areaRB(cx, cx + childWidth, cy - childHeight, cy);
			m_childs[right_bottom] = new Zone(areaRB, minZoneSize, this);

			Area areaRT(cx, cx + childWidth, cy, cy + childHeight);
			m_childs[right_top] = new Zone(areaRT, minZoneSize, this);
		}

	private:
		Area		m_area;
		Zone*		m_parent;
		Zone*		m_childs[4];

	private:
		BunchMap	m_bunchMap;
	};
}

#endif
