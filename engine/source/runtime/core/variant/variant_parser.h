//#pragma once
//#ifndef VARIANT_PARSER_H
//#define VARIANT_PARSER_H
//#include "variant.h"
//#include "core/io/resource.h"
//namespace lain {
//	class Resource;
//	
//	class VariantWriter {
//	public:
//		typedef Error(*StoreStringFunc)(void* ud, const String& p_string);
//		typedef String(*EncodeResourceFunc)(void* ud, const Ref<Resource>& p_resource);
//
//		static Error write(const Variant& p_variant, StoreStringFunc p_store_string_func, void* p_store_string_ud, EncodeResourceFunc p_encode_res_func, void* p_encode_res_ud, int p_recursion_count = 0);
//		static Error write_to_string(const Variant& p_variant, String& r_string, EncodeResourceFunc p_encode_res_func = nullptr, void* p_encode_res_ud = nullptr);
//	};
//	//static Error _write_to_str(void* ud, const String& p_string);
//
//}
//
//#endif