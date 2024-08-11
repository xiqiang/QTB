#ifndef QTB_QTree
#define QTB_QTree

#include <list>
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
			memset(m_children, 0, sizeof(m_children));
			m_hasChild = false;
		}

		virtual ~QTree() {
			for (int i = 0; i < CHILD_COUNT; ++i)
			{
				if (m_children[i])
					delete m_children[i];
			}
		}

	public:
		const Area& area() { return m_area; }

		void devide(float minSize) {
			assert(false == m_hasChild);

			float width = m_area.width() * 0.5f;
			float height = m_area.height() * 0.5f;

			if (width <= minSize || height <= minSize)
				return;

			float cx = m_area.x();
			float cy = m_area.y();

			Area areaLB(cx - width, cx, cy - height, cy);
			m_children[LB] = newChild(areaLB);
			m_children[LB]->devide(minSize);

			Area areaLT(cx - width, cx, cy, cy + height);
			m_children[LT] = newChild(areaLT);
			m_children[LT]->devide(minSize);

			Area areaRB(cx, cx + width, cy - height, cy);
			m_children[RB] = newChild(areaRB);
			m_children[RB]->devide(minSize);

			Area areaRT(cx, cx + width, cy, cy + height);
			m_children[RT] = newChild(areaRT);
			m_children[RT]->devide(minSize);

			m_hasChild = true;
		}

		QTree* child(unsigned int index) {
			assert(index < CHILD_COUNT);
			return m_children[index];
		}

		QTree* child(float x, float y) {
			if (!m_hasChild)
				return NULL;

			int ix = x - m_area.x() >= 0 ? 1 : 0;
			int iy = y - m_area.y() >= 0 ? 1 : 0;
			return m_children[(ix << 1) | iy];
		}

		QTree* locate(const Area& area) {
			QTree* c = child(area.x(), area.y());
			if (c && c->area().contains(area))
				return c->locate(area);
			else
				return this;
		}

		void layer(const Area& area, std::list<QTree*>& list) {
			if (!m_area.overlap(area))
				return;

			list.push_back(this);
			if (!m_hasChild)
				return;

			assert(m_children[LB]);
			m_children[LB]->layer(area, list);

			assert(m_children[LT]);
			m_children[LT]->layer(area, list);

			assert(m_children[RB]);
			m_children[RB]->layer(area, list);

			assert(m_children[RT]);
			m_children[RT]->layer(area, list);
		}

	protected:
		virtual QTree* newChild(const Area& area) = 0;

	protected:
		Area	m_area;
		QTree*	m_parent;
		QTree*	m_children[CHILD_COUNT];
		bool	m_hasChild;
	};
}

#endif
