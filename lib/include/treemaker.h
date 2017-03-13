#ifndef TREEMAKER_H
#define TREEMAKER_H

#include <cstddef>
#include "math.h"
#include "intersection.h"

namespace treemaker {

	class TreeImpl;

	class Tree {
		TreeImpl *d;

	public:
		Tree();
		~Tree();

		void generateTree();
		bool generateMesh();

		void setMaxStemDepth(unsigned depth);
		unsigned getMaxStemDepth();

		void setCrownBaseHeight(float height);
		float getCrownBaseHeight();

		unsigned getStemName(size_t index);
		
		unsigned newStem(unsigned parent);
		void deleteStem(unsigned stem);
		bool moveStem(unsigned stem, unsigned parent);

		bool isLateral(unsigned stem);
		
		/** 
		 * Positions can only be set for lateral stems. Positions are
		 * distances along the path of the stem's parent.
		 */
		void setPosition(unsigned stem, float position);
		float getPosition(unsigned stem);
		/** 
		 * Locations can only be set for non-lateral stems 
		 * (e.g. trunks).
		 */
		void setLocation(unsigned stem, Vec3 location);
		Vec3 getLocation(unsigned stem);

		void setRadius(unsigned stem, float radius);
		float getRadius(unsigned stem);

		void setRadiusCurve(unsigned stem, Vec3 *curve, size_t size);
		size_t getRadiusCurveSize(unsigned stem);
		void getRadiusCurve(unsigned stem, Vec3 *curve);

		void setStemDensity(unsigned stem, float density);
		float getStemDensity(unsigned stem);

		void setResolution(unsigned stem, int resolution);
		int getResolution(unsigned stem);

		/** The first point of any path is always a zero vector. */
		void setPath(unsigned stem, Vec3 *path, size_t size);
		size_t getPathSize(unsigned stem);
		void getPath(unsigned stem, Vec3 *path);

		size_t getGeneratedPathSize(unsigned stem);
		/** 
		 * The generated path will be a different shape depending on the
		 * number of cross sections and represents the actual path the
		 * stem will follow.
		 */
		void getGeneratedPath(unsigned stem, Vec3 *path);
		void setGeneratedPathSize(unsigned stem, size_t count);

		const float *getVertices();
		size_t getVertexCount();
		size_t getVertexCapacity();
		const unsigned *getIndices();
		size_t getIndexCount();
		size_t getIndexCapacity();

		struct Location {
			size_t indexStart;
			size_t indexCount;
		};

		Location getStemLocation(unsigned stem);
		Aabb getBoundingBox(unsigned stem);
	};
}

#endif /* TREEMAKER_H */
