#pragma once
#ifndef VARIANT_PARSER_H
#define VARIANT_PARSER_H

namespace lain {
	class VariantWriter {
		static Error VariantWriter::_write_to_str(void* ud, const String& p_string) {

		}

	}
static Error _write_to_str(void* ud, const String& p_string) {
	String* str = (String*)ud;
	(*str) += p_string;
	return OK;
}

Error VariantWriter::write_to_string(const Variant& p_variant, String& r_string, EncodeResourceFunc p_encode_res_func, void* p_encode_res_ud) {
	r_string = String();

	return write(p_variant, _write_to_str, &r_string, p_encode_res_func, p_encode_res_ud);
}
}

#endif