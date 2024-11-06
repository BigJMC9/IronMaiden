#pragma once
#include "../Asset/Asset.h"

namespace Madam
{	
	using ResourceDescriptorInfo = void*;

	class RenderResource : public Asset
	{
	public:
		virtual ResourceDescriptorInfo GetDescriptorInfo() const = 0;
	};
}