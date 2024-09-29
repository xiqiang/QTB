#ifndef QTB_QTree
#define QTB_QTree

#include <list>
#include <cassert>
#include "Area.h"
#include "Util.h"

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
			m_generation = m_parent ? m_parent->generation() + 1 : 0;
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
		const Area& area() const { return m_area; }
		unsigned int generation() const { return m_generation; }

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
			if (m_children[LB])
			{
				m_hasChild = true;
				m_children[LB]->devide(minSize);
			}

			Area areaLT(cx - width, cx, cy, cy + height);
			m_children[LT] = newChild(areaLT);
			if (m_children[LT])
			{
				m_hasChild = true;
				m_children[LT]->devide(minSize);
			}

			Area areaRB(cx, cx + width, cy - height, cy);
			m_children[RB] = newChild(areaRB);
			if (m_children[RB])
			{
				m_hasChild = true;
				m_children[RB]->devide(minSize);
			}

			Area areaRT(cx, cx + width, cy, cy + height);
			m_children[RT] = newChild(areaRT);
			if (m_children[RT])
			{
				m_hasChild = true;
				m_children[RT]->devide(minSize);
			}
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

		bool layer(const Area& area, std::list<QTree*>& list) {
			if (!m_area.overlap(area))
				return true;

			if (!layerTest())
				return true;
			
			try
			{
				QTB_RAND_BAD_ALLOC(1);
				list.push_back(this);
			}
			catch (std::bad_alloc&)
			{
				qtbLog("bad_alloc:  QTree::layer\n");
				return false;
			}

			if (!m_hasChild)
				return true;

			if (m_children[LB] && !m_children[LB]->layer(area, list))
				return false;

			if (m_children[LT] && !m_children[LT]->layer(area, list))
				return false;

			if (m_children[RB] && !m_children[RB]->layer(area, list))
				return false;

			if (m_children[RT] && !m_children[RT]->layer(area, list))
				return false;

			return true;
		}

	protected:
		virtual QTree*	newChild(const Area& area) = 0;
		virtual bool	layerTest() const { return true; }

	protected:
		Area			m_area;
		QTree*			m_parent;
		unsigned int	m_generation;
		QTree*			m_children[CHILD_COUNT];
		bool			m_hasChild;

		QTB_OVERLOAD_BLOCK
	};
}

#endif
