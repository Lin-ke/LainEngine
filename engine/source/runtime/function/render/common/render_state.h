#pragma once
#ifndef RENDER_STATE_H
#define RENDER_STATE_H
#include "base.h"
namespace lain {
	namespace graphics {
		enum BlendMode // High level wrappers. Order is important (see renderstate.cpp)
		{
			BLEND_ALPHA,
			BLEND_ADD,
			BLEND_SUBTRACT,
			BLEND_MULTIPLY,
			BLEND_LIGHTEN,
			BLEND_DARKEN,
			BLEND_SCREEN,
			BLEND_REPLACE,
			BLEND_NONE,
			BLEND_CUSTOM,
			BLEND_MAX_ENUM
		};

		enum BlendAlpha // High level wrappers
		{
			BLENDALPHA_MULTIPLY,
			BLENDALPHA_PREMULTIPLIED,
			BLENDALPHA_MAX_ENUM
		};

		enum BlendFactor
		{
			BLENDFACTOR_ZERO,
			BLENDFACTOR_ONE,
			BLENDFACTOR_SRC_COLOR,
			BLENDFACTOR_ONE_MINUS_SRC_COLOR,
			BLENDFACTOR_SRC_ALPHA,
			BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			BLENDFACTOR_DST_COLOR,
			BLENDFACTOR_ONE_MINUS_DST_COLOR,
			BLENDFACTOR_DST_ALPHA,
			BLENDFACTOR_ONE_MINUS_DST_ALPHA,
			BLENDFACTOR_SRC_ALPHA_SATURATED,
			BLENDFACTOR_MAX_ENUM
		};

		enum BlendOperation
		{
			BLENDOP_ADD,
			BLENDOP_SUBTRACT,
			BLENDOP_REVERSE_SUBTRACT,
			BLENDOP_MIN,
			BLENDOP_MAX,
			BLENDOP_MAX_ENUM
		};

		enum StencilAction
		{
			STENCIL_KEEP,
			STENCIL_ZERO,
			STENCIL_REPLACE,
			STENCIL_INCREMENT,
			STENCIL_DECREMENT,
			STENCIL_INCREMENT_WRAP,
			STENCIL_DECREMENT_WRAP,
			STENCIL_INVERT,
			STENCIL_MAX_ENUM
		};
		enum CompareMode
		{
			COMPARE_LESS,
			COMPARE_LEQUAL,
			COMPARE_EQUAL,
			COMPARE_GEQUAL,
			COMPARE_GREATER,
			COMPARE_NOTEQUAL,
			COMPARE_ALWAYS,
			COMPARE_NEVER,
			COMPARE_MAX_ENUM
		};

		struct BlendState
		{
			BlendOperation operationRGB = BLENDOP_ADD;
			BlendOperation operationA = BLENDOP_ADD;
			BlendFactor srcFactorRGB = BLENDFACTOR_ONE;
			BlendFactor srcFactorA = BLENDFACTOR_ONE;
			BlendFactor dstFactorRGB = BLENDFACTOR_ZERO;
			BlendFactor dstFactorA = BLENDFACTOR_ZERO;
			bool enable = false;

			BlendState() {}

			BlendState(BlendOperation opRGB, BlendOperation opA, BlendFactor srcRGB, BlendFactor srcA, BlendFactor dstRGB, BlendFactor dstA)
				: operationRGB(opRGB)
				, operationA(opA)
				, srcFactorRGB(srcRGB)
				, srcFactorA(srcA)
				, dstFactorRGB(dstRGB)
				, dstFactorA(dstA)
				, enable(true)
			{}

			bool operator == (const BlendState& b) const
			{
				return enable == b.enable
					&& operationRGB == b.operationRGB && operationA == b.operationA
					&& srcFactorRGB == b.srcFactorRGB && srcFactorA == b.srcFactorA
					&& dstFactorRGB == b.dstFactorRGB && dstFactorA == b.dstFactorA;
			}
		};

		struct DepthState
		{
			CompareMode compare = COMPARE_ALWAYS;
			bool write = false;

			bool operator == (const DepthState& d) const
			{
				return compare == d.compare && write == d.write;
			}
		};

		struct StencilState
		{
			CompareMode compare = COMPARE_ALWAYS;
			StencilAction action = STENCIL_KEEP;
			int value = 0;
			uint32 readMask = 0xFFFFFFFF;
			uint32 writeMask = 0xFFFFFFFF;

			bool operator == (const StencilState& s) const
			{
				return compare == s.compare && action == s.action && value == s.value
					&& readMask == s.readMask && writeMask == s.writeMask;
			}
		};

	}
}

#endif