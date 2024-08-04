#ifndef TreeBush_QTree
#define TreeBush_QTree

#include <cassert>
#include "Area.h"

namespace treebush 
{
	template <class T>
	class QTree
	{
	public:
		static const int left_bottom	= 0;
		static const int left_top		= 1;
		static const int right_bottom	= 2;
		static const int right_top		= 3;

	public:
		QTree(const Area& area, float minQTreeSize, T* parent = NULL) {
			m_area = area;
			m_parent = parent;
			memset(m_childs, 0, sizeof(m_childs));
			devide(minQTreeSize);
		}

	public:
		const Area& area() { return m_area; }

		void devide(float minQTreeSize) {
			float childWidth = m_area.width() * 0.5;
			float childHeight = m_area.height() * 0.5;

			if (childWidth <= minQTreeSize || childHeight <= minQTreeSize)
				return;

			float cx = m_area.x();
			float cy = m_area.y();

			Area areaLB(cx - childWidth, cx, cy - childHeight, cy);
			m_childs[left_bottom] = new T(areaLB, minQTreeSize, this);

			Area areaLT(cx - childWidth, cx, cy, cy + childHeight);
			m_childs[left_top] = new T(areaLT, minQTreeSize, this);

			Area areaRB(cx, cx + childWidth, cy - childHeight, cy);
			m_childs[right_bottom] = new T(areaRB, minQTreeSize, this);

			Area areaRT(cx, cx + childWidth, cy, cy + childHeight);
			m_childs[right_top] = new T(areaRT, minQTreeSize, this);
		}

	protected:
		Area	m_area;
		T*		m_parent;
		T*		m_childs[4];
	};
}

#endif
