#pragma once
#include "maidenpch.hpp"

#define INCLUDE_UTILS
#include "../Utils/H_Utils.h"

#include "H_RenderUtils.hpp"
#include "H_RIResource.h"
#include "H_Renderer.hpp"

namespace Madam {
	namespace Rendering {
		class Renderer;
		class Image : public RenderResource
		{
		public:
			enum class Format
			{
				None = 0,
				RGB,
				RGB_SRGB,
				RGB16F,
				RGB32F,
				RGBA,
				RGBA_SRGB,
				RGBA16F,
				RGBA32F,
				BGRA,
				BGRA_SRGB,
				D32F,
				D32FS8,
				D24S8
			};

			enum class Interpolation
			{
				NEAREST = 0,
				BILINEAR,
				CUBIC,
				TRILINEAR,
			};

			enum class Multisample
			{
				None = 0,
				MSAAx2,
				MSAAx4,
				MSAAx8
			};

			enum class Usage
			{
				None = 0,
				Texture,
				Attachment,
				Storage,
				HostRead
			};

			virtual ~Image() = default;

			virtual void Resize(const uint32_t width, const uint32_t height) = 0;
			virtual void Resize(const uint32_t width, const uint32_t height, const uint32_t depth) = 0;

			virtual float GetAspectRatio() const = 0;

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			virtual uint32_t GetDepth() const = 0;

			struct ImageData {
				Format format = Format::RGBA;
				uint32_t width = 0, height = 0, depth = 1;
				uint32_t mipLevels = 1, arrayLayers = 1;
				Multisample multisample = Multisample::None;
				Usage usage = Usage::None;
			};

		protected:
			ImageData data;
			friend class SwapChain;
			friend class Renderer;
		};
	}
}