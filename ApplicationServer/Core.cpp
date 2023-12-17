#include "Core.h"

std::string GibCore::GetImageFilterName(GibCore::ImageFilter filter)
{
	switch (filter)
	{
	case GibCore::ImageFilter::RESIZE:
	{
		return std::string("Resize");
	}
	case GibCore::ImageFilter::ROTATION:
		return std::string("Rotation");
	default:
		return std::string("Unknown");
	}
}
