#ifndef TreeBush_Area
#define TreeBush_Area

#include <list>
#include <vector>

namespace qtb
{
	struct Area
	{
		Area()
			: left(0)
			, right(0)
			, bottom(0)
			, top(0)
		{}

		Area(float _left, float _right, float _bottom, float _top)
			: left(_left)
			, right(_right)
			, bottom(_bottom)
			, top(_top)
		{
			assert(left <= right);
			assert(bottom <= top);
		}

		float x() const { return  (left + right) * 0.5f; }
		float y() const { return  (bottom + top) * 0.5f; }

		float width() const { return right - left; }
		float height() const { return top - bottom; }

		bool overlap(const Area& r) const {
			return left <= r.right && right >= r.left 
				&& bottom <= r.top && top >= r.bottom;
		}

		bool contains(const Area& r) const {
			return left <= r.left && right >= r.right
				&& bottom <= r.bottom && top >= r.top;
		}

		bool cross(float x, float y) const {
			return x >= left && x <= right 
				&& y >= bottom && y <= top;
		}

		float left;
		float right;
		float bottom;
		float top;
	};

	typedef std::list<Area>		AreaList;
	typedef std::vector<Area>	AreaVector;

}

#endif
