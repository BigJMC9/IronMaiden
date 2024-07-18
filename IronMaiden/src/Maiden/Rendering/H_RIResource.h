#pragma once
#include "../Asset/Asset.h"

namespace Madam
{
	enum class ResourceType
	{
		UnDefined = 0,
		Texture
	};
	
	using ResourceDescriptorInfo = void*;

	class RenderResource : public Asset
	{
	public:
		virtual ResourceDescriptorInfo GetDescriptorInfo() const = 0;
		static ResourceType GetStaticType() { return ResourceType::UnDefined; }
		virtual ResourceType GetResourceType() const { return GetStaticType(); }
	};
}