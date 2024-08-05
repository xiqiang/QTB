#ifndef TreeBush_QTree
#define TreeBush_QTree

#include <cassert>
#include "Area.h"

namespace qtb 
{
	class QTree
	{
	public:
		static const int LB	= 0;
		static const int LT	= 1;
		static const int RB	= 2;
		static const int RT	= 3;

		static const int CHILD_COUNT = 4;

	public:
		QTree(const Area& area, QTree* parent = NULL) {
			m_area = area;
			m_parent = parent;
			memset(m_childs, 0, sizeof(m_childs));
		}

		virtual ~QTree() {
			for (int i = 0; i < CHILD_COUNT; ++i)
			{
				if (m_childs[i])
					delete m_childs[i];
			}
		}

		virtual QTree*	newChild(const Area& area) = 0;

		QTree* getChild(unsigned int index) {
			assert(index < CHILD_COUNT);
			return m_childs[index];
		}

	public:
		const Area& area() { return m_area; }

		void devide(float minQTreeSize) {
			float childWidth = m_area.width() * 0.5f;
			float childHeight = m_area.height() * 0.5f;

			if (childWidth <= minQTreeSize || childHeight <= minQTreeSize)
				return;

			float cx = m_area.x();
			float cy = m_area.y();

			Area areaLB(cx - childWidth, cx, cy - childHeight, cy);
			m_childs[LB] = newChild(areaLB);
			m_childs[LB]->devide(minQTreeSize);

			Area areaLT(cx - childWidth, cx, cy, cy + childHeight);
			m_childs[LT] = newChild(areaLT);
			m_childs[LT]->devide(minQTreeSize);

			Area areaRB(cx, cx + childWidth, cy - childHeight, cy);
			m_childs[RB] = newChild(areaRB);
			m_childs[RB]->devide(minQTreeSize);

			Area areaRT(cx, cx + childWidth, cy, cy + childHeight);
			m_childs[RT] = newChild(areaRT);
			m_childs[RT]->devide(minQTreeSize);
		}

	protected:
		Area	m_area;
		QTree*	m_parent;
		QTree*	m_childs[CHILD_COUNT];
	};
}

#endif
