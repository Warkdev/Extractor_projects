#include "C4Plane.h"

C4Plane::C4Plane() : normal(0, 0, 0), distance(distance)
{
}

C4Plane::C4Plane(float x, float y, float z, float distance) : normal(x, y, z), distance(distance)
{
}

std::ostream& operator<<(std::ostream& stream, const C4Plane& plane)
{
	stream << plane.normal << plane.distance;
	return stream;
}

std::istream& operator>>(std::istream& stream, C4Plane& plane)
{
	stream >> plane.normal >> plane.distance;
	return stream;
}
