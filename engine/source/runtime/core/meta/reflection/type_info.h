#pragma once
#include "reflection.h"
#include "core/variant/variant.h"
#include "core/string/string_name.h"
namespace lain {

template <class T, typename = void>
struct GetTypeInfo;
// 类和某个属性都用这个数据结构
struct PropertyInfo {
public:
	Variant::Type type;
	String name;
	StringName class_name;
	PropertyInfo(const Variant::Type p_type, const String p_name, const StringName& p_class_name = StringName()) :
		type(p_type),
		name(p_name)
	{
	class_name = p_class_name; // temporary
	}
};
namespace typeinfo {
	enum Metadata {
		METADATA_NONE,
		METADATA_INT_IS_INT8,
		METADATA_INT_IS_INT16,
		METADATA_INT_IS_INT32,
		METADATA_INT_IS_INT64,
		METADATA_INT_IS_UINT8,
		METADATA_INT_IS_UINT16,
		METADATA_INT_IS_UINT32,
		METADATA_INT_IS_UINT64,
		METADATA_REAL_IS_FLOAT,
		METADATA_REAL_IS_DOUBLE
	};
}
//带有类内初始值设定项的成员必须为常量（且为integral types）；

#define MAKE_TYPE_INFO(m_type, m_var_type)                                            \
	template <>                                                                       \
	struct GetTypeInfo<m_type> {                                                      \
		static const Variant::Type VARIANT_TYPE = m_var_type;                         \
		static const typeinfo::Metadata METADATA = typeinfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};                                                                                \
	template <>                                                                       \
	struct GetTypeInfo<const m_type &> {                                              \
		static const Variant::Type VARIANT_TYPE = m_var_type;                         \
		static const typeinfo::Metadata METADATA = typeinfo::METADATA_NONE; \
		static inline PropertyInfo get_class_info() {                                 \
			return PropertyInfo(VARIANT_TYPE, String());                              \
		}                                                                             \
	};
#define MAKE_TYPE_INFO_WITH_META(m_type, m_var_type, m_metadata)    \
	template <>                                                     \
	struct GetTypeInfo<m_type> {                                    \
		static const Variant::Type VARIANT_TYPE = m_var_type;       \
		static const typeinfo::Metadata METADATA = m_metadata; \
		static inline PropertyInfo get_class_info() {               \
			return PropertyInfo(VARIANT_TYPE, String());            \
		}                                                           \
	};                                                              \
	template <>                                                     \
	struct GetTypeInfo<const m_type &> {                            \
		static const Variant::Type VARIANT_TYPE = m_var_type;       \
		static const typeinfo::Metadata METADATA = m_metadata; \
		static inline PropertyInfo get_class_info() {               \
			return PropertyInfo(VARIANT_TYPE, String());            \
		}                                                           \
	};

 
MAKE_TYPE_INFO(bool, Variant::Type::BOOL);
MAKE_TYPE_INFO_WITH_META(uint8_t, Variant::INT, typeinfo::METADATA_INT_IS_UINT8)
MAKE_TYPE_INFO_WITH_META(int8_t, Variant::INT, typeinfo::METADATA_INT_IS_INT8)
MAKE_TYPE_INFO_WITH_META(uint16_t, Variant::INT, typeinfo::METADATA_INT_IS_UINT16)
MAKE_TYPE_INFO_WITH_META(int16_t, Variant::INT, typeinfo::METADATA_INT_IS_INT16)
MAKE_TYPE_INFO_WITH_META(uint32_t, Variant::INT, typeinfo::METADATA_INT_IS_UINT32)
MAKE_TYPE_INFO_WITH_META(int32_t, Variant::INT, typeinfo::METADATA_INT_IS_INT32)
MAKE_TYPE_INFO_WITH_META(uint64_t, Variant::INT, typeinfo::METADATA_INT_IS_UINT64)
MAKE_TYPE_INFO_WITH_META(int64_t, Variant::INT, typeinfo::METADATA_INT_IS_INT64)
MAKE_TYPE_INFO(char16_t, Variant::INT)
MAKE_TYPE_INFO(char32_t, Variant::INT)
MAKE_TYPE_INFO_WITH_META(float, Variant::FLOAT, typeinfo::METADATA_REAL_IS_FLOAT)
MAKE_TYPE_INFO_WITH_META(double, Variant::FLOAT, typeinfo::METADATA_REAL_IS_DOUBLE)

}
