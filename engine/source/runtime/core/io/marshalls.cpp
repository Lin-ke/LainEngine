#include "marshalls.h"
#include "core/object/object.h"
#include "core/meta/class_db.h"
namespace lain{


// Byte 0: `Variant::Type`, byte 1: unused, bytes 2 and 3: additional data.
#define HEADER_TYPE_MASK 0xFF
// For `Variant::INT`, `Variant::FLOAT` and other math types.
#define HEADER_DATA_FLAG_64 (1 << 16)

// For `Variant::OBJECT`.
#define HEADER_DATA_FLAG_OBJECT_AS_ID (1 << 16)

// For `Variant::ARRAY`.
// Occupies bits 16 and 17.
#define HEADER_DATA_FIELD_TYPED_ARRAY_MASK (0b11 << 16)
#define HEADER_DATA_FIELD_TYPED_ARRAY_NONE (0b00 << 16)
#define HEADER_DATA_FIELD_TYPED_ARRAY_BUILTIN (0b01 << 16)
#define HEADER_DATA_FIELD_TYPED_ARRAY_CLASS_NAME (0b10 << 16)
#define HEADER_DATA_FIELD_TYPED_ARRAY_SCRIPT (0b11 << 16)

// extra error
#define ERR_FAIL_ADD_OF(a, b, err) ERR_FAIL_COND_V(((int32_t)(b)) < 0 || ((int32_t)(a)) < 0 || ((int32_t)(a)) > INT_MAX - ((int32_t)(b)), err)
#define ERR_FAIL_MUL_OF(a, b, err) ERR_FAIL_COND_V(((int32_t)(a)) < 0 || ((int32_t)(b)) <= 0 || ((int32_t)(a)) > INT_MAX / ((int32_t)(b)), err)

static Error _decode_string(const uint8_t *&buf, int &len, int *r_len, String &r_string) {
	ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);

	int32_t strlen = decode_uint32(buf);
	int32_t pad = 0;

	// Handle padding
	if (strlen % 4) {
		pad = 4 - strlen % 4;
	}

	buf += 4;
	len -= 4;

	// Ensure buffer is big enough
	ERR_FAIL_ADD_OF(strlen, pad, ERR_FILE_EOF);
	ERR_FAIL_COND_V(strlen < 0 || strlen + pad > len, ERR_FILE_EOF);

	String str;
	ERR_FAIL_COND_V(str.parse_utf8((const char *)buf, strlen) != OK, ERR_INVALID_DATA);
	r_string = str;

	// Add padding
	strlen += pad;

	// Update buffer pos, left data count, and return size
	buf += strlen;
	len -= strlen;
	if (r_len) {
		(*r_len) += 4 + strlen;
	}

	return OK;
}

Error decode_variant(Variant &r_variant, const uint8_t *p_buffer, int p_len, int *r_len, bool p_allow_objects, int p_depth)
{
    ERR_FAIL_COND_V_MSG(p_depth > Variant::MAX_RECURSION_DEPTH, ERR_OUT_OF_MEMORY, "Variant is too deep. Bailing.");
    const uint8_t *buf = p_buffer;
    int len = p_len;

    ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);

    uint32_t header = decode_uint32(buf);

    ERR_FAIL_COND_V((header & HEADER_TYPE_MASK) >= Variant::VARIANT_MAX, ERR_INVALID_DATA);

    buf += 4;
    len -= 4;
    if (r_len)
    {
        *r_len = 4;
    }
    switch (header & HEADER_TYPE_MASK)
    {
    case Variant::NIL:
    {
        r_variant = Variant();
    }
    break;
    case Variant::BOOL:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        bool val = decode_uint32(buf);
        r_variant = val;
        if (r_len)
        {
            (*r_len) += 4;
        }
    }
    break;
    case Variant::INT:
    {
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V(len < 8, ERR_INVALID_DATA);
            int64_t val = decode_uint64(buf);
            r_variant = val;
            if (r_len)
            {
                (*r_len) += 8;
            }
        }
        else
        {
            ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
            int32_t val = decode_uint32(buf);
            r_variant = val;
            if (r_len)
            {
                (*r_len) += 4;
            }
        }
    }
    break;
    case Variant::FLOAT:
    {
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double), ERR_INVALID_DATA);
            double val = decode_double(buf);
            r_variant = val;
            if (r_len)
            {
                (*r_len) += sizeof(double);
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float), ERR_INVALID_DATA);
            float val = decode_float(buf);
            r_variant = val;
            if (r_len)
            {
                (*r_len) += sizeof(float);
            }
        }
    }
    break;
    case Variant::STRING:
    {
        String str;
        Error err = _decode_string(buf, len, r_len, str);
        if (err)
        {
            return err;
        }
        r_variant = str;
    }
    break;

    // math types
    case Variant::VECTOR2:
    {
        Vector2 val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 2, ERR_INVALID_DATA);
            val.x = decode_double(&buf[0]);
            val.y = decode_double(&buf[sizeof(double)]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 2;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 2, ERR_INVALID_DATA);
            val.x = decode_float(&buf[0]);
            val.y = decode_float(&buf[sizeof(float)]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 2;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::VECTOR2I:
    {
        ERR_FAIL_COND_V(len < 4 * 2, ERR_INVALID_DATA);
        Vector2i val;
        val.x = decode_uint32(&buf[0]);
        val.y = decode_uint32(&buf[4]);
        r_variant = val;

        if (r_len)
        {
            (*r_len) += 4 * 2;
        }
    }
    break;
    case Variant::RECT2:
    {
        Rect2 val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 4, ERR_INVALID_DATA);
            val.position.x = decode_double(&buf[0]);
            val.position.y = decode_double(&buf[sizeof(double)]);
            val.size.x = decode_double(&buf[sizeof(double) * 2]);
            val.size.y = decode_double(&buf[sizeof(double) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 4;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 4, ERR_INVALID_DATA);
            val.position.x = decode_float(&buf[0]);
            val.position.y = decode_float(&buf[sizeof(float)]);
            val.size.x = decode_float(&buf[sizeof(float) * 2]);
            val.size.y = decode_float(&buf[sizeof(float) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 4;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::RECT2I:
    {
        ERR_FAIL_COND_V(len < 4 * 4, ERR_INVALID_DATA);
        Rect2i val;
        val.position.x = decode_uint32(&buf[0]);
        val.position.y = decode_uint32(&buf[4]);
        val.size.x = decode_uint32(&buf[8]);
        val.size.y = decode_uint32(&buf[12]);
        r_variant = val;

        if (r_len)
        {
            (*r_len) += 4 * 4;
        }
    }
    break;
    case Variant::VECTOR3:
    {
        Vector3 val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 3, ERR_INVALID_DATA);
            val.x = decode_double(&buf[0]);
            val.y = decode_double(&buf[sizeof(double)]);
            val.z = decode_double(&buf[sizeof(double) * 2]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 3;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 3, ERR_INVALID_DATA);
            val.x = decode_float(&buf[0]);
            val.y = decode_float(&buf[sizeof(float)]);
            val.z = decode_float(&buf[sizeof(float) * 2]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 3;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::VECTOR3I:
    {
        ERR_FAIL_COND_V(len < 4 * 3, ERR_INVALID_DATA);
        Vector3i val;
        val.x = decode_uint32(&buf[0]);
        val.y = decode_uint32(&buf[4]);
        val.z = decode_uint32(&buf[8]);
        r_variant = val;

        if (r_len)
        {
            (*r_len) += 4 * 3;
        }
    }
    break;
    case Variant::VECTOR4:
    {
        Vector4 val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 4, ERR_INVALID_DATA);
            val.x = decode_double(&buf[0]);
            val.y = decode_double(&buf[sizeof(double)]);
            val.z = decode_double(&buf[sizeof(double) * 2]);
            val.w = decode_double(&buf[sizeof(double) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 4;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 4, ERR_INVALID_DATA);
            val.x = decode_float(&buf[0]);
            val.y = decode_float(&buf[sizeof(float)]);
            val.z = decode_float(&buf[sizeof(float) * 2]);
            val.w = decode_float(&buf[sizeof(float) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 4;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::VECTOR4I:
    {
        ERR_FAIL_COND_V(len < 4 * 4, ERR_INVALID_DATA);
        Vector4i val;
        val.x = decode_uint32(&buf[0]);
        val.y = decode_uint32(&buf[4]);
        val.z = decode_uint32(&buf[8]);
        val.w = decode_uint32(&buf[12]);
        r_variant = val;

        if (r_len)
        {
            (*r_len) += 4 * 4;
        }
    }
    break;
    // case Variant::TRANSFORM2D:
    // {
    //     Transform2D val;
    //     if (header & HEADER_DATA_FLAG_64)
    //     {
    //         ERR_FAIL_COND_V((size_t)len < sizeof(double) * 6, ERR_INVALID_DATA);
    //         for (int i = 0; i < 3; i++)
    //         {
    //             for (int j = 0; j < 2; j++)
    //             {
    //                 val.columns[i][j] = decode_double(&buf[(i * 2 + j) * sizeof(double)]);
    //             }
    //         }

    //         if (r_len)
    //         {
    //             (*r_len) += sizeof(double) * 6;
    //         }
    //     }
    //     else
    //     {
    //         ERR_FAIL_COND_V((size_t)len < sizeof(float) * 6, ERR_INVALID_DATA);
    //         for (int i = 0; i < 3; i++)
    //         {
    //             for (int j = 0; j < 2; j++)
    //             {
    //                 val.columns[i][j] = decode_float(&buf[(i * 2 + j) * sizeof(float)]);
    //             }
    //         }

    //         if (r_len)
    //         {
    //             (*r_len) += sizeof(float) * 6;
    //         }
    //     }
    //     r_variant = val;
    // }
    // break;
    case Variant::PLANE:
    {
        Plane val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 4, ERR_INVALID_DATA);
            val.normal.x = decode_double(&buf[0]);
            val.normal.y = decode_double(&buf[sizeof(double)]);
            val.normal.z = decode_double(&buf[sizeof(double) * 2]);
            val.d = decode_double(&buf[sizeof(double) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 4;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 4, ERR_INVALID_DATA);
            val.normal.x = decode_float(&buf[0]);
            val.normal.y = decode_float(&buf[sizeof(float)]);
            val.normal.z = decode_float(&buf[sizeof(float) * 2]);
            val.d = decode_float(&buf[sizeof(float) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 4;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::QUATERNION:
    {
        Quaternion val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 4, ERR_INVALID_DATA);
            val.x = decode_double(&buf[0]);
            val.y = decode_double(&buf[sizeof(double)]);
            val.z = decode_double(&buf[sizeof(double) * 2]);
            val.w = decode_double(&buf[sizeof(double) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 4;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 4, ERR_INVALID_DATA);
            val.x = decode_float(&buf[0]);
            val.y = decode_float(&buf[sizeof(float)]);
            val.z = decode_float(&buf[sizeof(float) * 2]);
            val.w = decode_float(&buf[sizeof(float) * 3]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 4;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::AABB:
    {
        AABB val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 6, ERR_INVALID_DATA);
            val.position.x = decode_double(&buf[0]);
            val.position.y = decode_double(&buf[sizeof(double)]);
            val.position.z = decode_double(&buf[sizeof(double) * 2]);
            val.size.x = decode_double(&buf[sizeof(double) * 3]);
            val.size.y = decode_double(&buf[sizeof(double) * 4]);
            val.size.z = decode_double(&buf[sizeof(double) * 5]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 6;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 6, ERR_INVALID_DATA);
            val.position.x = decode_float(&buf[0]);
            val.position.y = decode_float(&buf[sizeof(float)]);
            val.position.z = decode_float(&buf[sizeof(float) * 2]);
            val.size.x = decode_float(&buf[sizeof(float) * 3]);
            val.size.y = decode_float(&buf[sizeof(float) * 4]);
            val.size.z = decode_float(&buf[sizeof(float) * 5]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 6;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::BASIS:
    {
        Basis val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 9, ERR_INVALID_DATA);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    val.rows[i][j] = decode_double(&buf[(i * 3 + j) * sizeof(double)]);
                }
            }

            if (r_len)
            {
                (*r_len) += sizeof(double) * 9;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 9, ERR_INVALID_DATA);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    val.rows[i][j] = decode_float(&buf[(i * 3 + j) * sizeof(float)]);
                }
            }

            if (r_len)
            {
                (*r_len) += sizeof(float) * 9;
            }
        }
        r_variant = val;
    }
    break;
    case Variant::TRANSFORM3D:
    {
        Transform3D val;
        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(double) * 12, ERR_INVALID_DATA);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    val.basis.rows[i][j] = decode_double(&buf[(i * 3 + j) * sizeof(double)]);
                }
            }
            val.origin[0] = decode_double(&buf[sizeof(double) * 9]);
            val.origin[1] = decode_double(&buf[sizeof(double) * 10]);
            val.origin[2] = decode_double(&buf[sizeof(double) * 11]);

            if (r_len)
            {
                (*r_len) += sizeof(double) * 12;
            }
        }
        else
        {
            ERR_FAIL_COND_V((size_t)len < sizeof(float) * 12, ERR_INVALID_DATA);
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    val.basis.rows[i][j] = decode_float(&buf[(i * 3 + j) * sizeof(float)]);
                }
            }
            val.origin[0] = decode_float(&buf[sizeof(float) * 9]);
            val.origin[1] = decode_float(&buf[sizeof(float) * 10]);
            val.origin[2] = decode_float(&buf[sizeof(float) * 11]);

            if (r_len)
            {
                (*r_len) += sizeof(float) * 12;
            }
        }
        r_variant = val;
    }
    break;
    // case Variant::PROJECTION:
    // {
    //     Projection val;
    //     if (header & HEADER_DATA_FLAG_64)
    //     {
    //         ERR_FAIL_COND_V((size_t)len < sizeof(double) * 16, ERR_INVALID_DATA);
    //         for (int i = 0; i < 4; i++)
    //         {
    //             for (int j = 0; j < 4; j++)
    //             {
    //                 val.columns[i][j] = decode_double(&buf[(i * 4 + j) * sizeof(double)]);
    //             }
    //         }
    //         if (r_len)
    //         {
    //             (*r_len) += sizeof(double) * 16;
    //         }
    //     }
    //     else
    //     {
    //         ERR_FAIL_COND_V((size_t)len < sizeof(float) * 16, ERR_INVALID_DATA);
    //         for (int i = 0; i < 4; i++)
    //         {
    //             for (int j = 0; j < 4; j++)
    //             {
    //                 val.columns[i][j] = decode_float(&buf[(i * 4 + j) * sizeof(float)]);
    //             }
    //         }

    //         if (r_len)
    //         {
    //             (*r_len) += sizeof(float) * 16;
    //         }
    //     }
    //     r_variant = val;
    // }
    // break;
    // misc types
    case Variant::COLOR:
    {
        ERR_FAIL_COND_V(len < 4 * 4, ERR_INVALID_DATA);
        Color val;
        val.r = decode_float(&buf[0]);
        val.g = decode_float(&buf[4]);
        val.b = decode_float(&buf[8]);
        val.a = decode_float(&buf[12]);
        r_variant = val;

        if (r_len)
        {
            (*r_len) += 4 * 4; // Colors should always be in single-precision.
        }
    }
    break;
    case Variant::STRING_NAME:
    {
        String str;
        Error err = _decode_string(buf, len, r_len, str);
        if (err)
        {
            return err;
        }
        r_variant = StringName(str);
    }
    break;

    case Variant::GOBJECT_PATH:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t strlen = decode_uint32(buf);

        if (strlen & 0x80000000)
        {
            // new format
            ERR_FAIL_COND_V(len < 12, ERR_INVALID_DATA);
            Vector<StringName> names;
            Vector<StringName> subnames;

            uint32_t namecount = strlen &= 0x7FFFFFFF;
            uint32_t subnamecount = decode_uint32(buf + 4);
            uint32_t np_flags = decode_uint32(buf + 8);

            len -= 12;
            buf += 12;

            if (np_flags & 2)
            { // Obsolete format with property separate from subpath.
                subnamecount++;
            }

            uint32_t total = namecount + subnamecount;

            if (r_len)
            {
                (*r_len) += 12;
            }

            for (uint32_t i = 0; i < total; i++)
            {
                String str;
                Error err = _decode_string(buf, len, r_len, str);
                if (err)
                {
                    return err;
                }

                if (i < namecount)
                {
                    names.push_back(str);
                }
                else
                {
                    subnames.push_back(str);
                }
            }

            r_variant = GObjectPath(names, subnames, np_flags & 1);
        }
        else
        {
            // old format, just a string

            ERR_FAIL_V(ERR_INVALID_DATA);
        }
    }
    break;
    case Variant::RID:
    {
        ERR_FAIL_COND_V(len < 8, ERR_INVALID_DATA);
        uint64_t id = decode_uint64(buf);
        if (r_len)
        {
            (*r_len) += 8;
        }

        r_variant = RID::from_uint64(id);
    }
    break;
    case Variant::OBJECT:
    {
        if (header & HEADER_DATA_FLAG_OBJECT_AS_ID)
        {
            // This _is_ allowed.
            ERR_FAIL_COND_V(len < 8, ERR_INVALID_DATA);
            ObjectID val = ObjectID(decode_uint64(buf));
            if (r_len)
            {
                (*r_len) += 8;
            }

            if (val.is_null())
            {
                r_variant = (Object *)nullptr;
            }
            else
            {
                // Ref<EncodedObjectAsID> obj_as_id;
                // obj_as_id.instantiate();
                // obj_as_id->set_object_id(val);

                // r_variant = obj_as_id;
            }
        }
    //     else
    //     {
    //         ERR_FAIL_COND_V(!p_allow_objects, ERR_UNAUTHORIZED);

    //         String str;
    //         Error err = _decode_string(buf, len, r_len, str);
    //         if (err)
    //         {
    //             return err;
    //         }

    //         if (str.is_empty())
    //         {
    //             r_variant = (Object *)nullptr;
    //         }
    //         else
    //         {
    //             ERR_FAIL_COND_V(!ClassDB::can_instantiate(str), ERR_INVALID_DATA);

    //             Object *obj = ClassDB::instantiate(str);
    //             ERR_FAIL_NULL_V(obj, ERR_UNAVAILABLE);

    //             // Avoid premature free `RefCounted`. This must be done before properties are initialized,
    //             // since script functions (setters, implicit initializer) may be called. See GH-68666.
    //             Variant variant;
    //             if (Object::cast_to<RefCounted>(obj))
    //             {
    //                 Ref<RefCounted> ref = Ref<RefCounted>(Object::cast_to<RefCounted>(obj));
    //                 variant = ref;
    //             }
    //             else
    //             {
    //                 variant = obj;
    //             }

    //             ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
    //             int32_t count = decode_uint32(buf);
    //             buf += 4;
    //             len -= 4;
    //             if (r_len)
    //             {
    //                 (*r_len) += 4; // Size of count number.
    //             }

    //             for (int i = 0; i < count; i++)
    //             {
    //                 str = String();
    //                 err = _decode_string(buf, len, r_len, str);
    //                 if (err)
    //                 {
    //                     return err;
    //                 }

    //                 Variant value;
    //                 int used;
    //                 err = decode_variant(value, buf, len, &used, p_allow_objects, p_depth + 1);
    //                 if (err)
    //                 {
    //                     return err;
    //                 }

    //                 buf += used;
    //                 len -= used;
    //                 if (r_len)
    //                 {
    //                     (*r_len) += used;
    //                 }

    //                 if (str == "script" && value.get_type() != Variant::NIL)
    //                 {
    //                     ERR_FAIL_COND_V_MSG(value.get_type() != Variant::STRING, ERR_INVALID_DATA, "Invalid value for \"script\" property, expected script path as String.");
    //                     String path = value;
    //                     ERR_FAIL_COND_V_MSG(path.is_empty() || !path.begins_with("res://") || !ResourceLoader::exists(path, "Script"), ERR_INVALID_DATA, "Invalid script path: '" + path + "'.");
    //                     Ref<Script> script = ResourceLoader::load(path, "Script");
    //                     ERR_FAIL_COND_V_MSG(script.is_null(), ERR_INVALID_DATA, "Can't load script at path: '" + path + "'.");
    //                     obj->set_script(script);
    //                 }
    //                 else
    //                 {
    //                     obj->set(str, value);
    //                 }
    //             }

    //             r_variant = variant;
    //         }
    //     }
    // }
    else {
        // encode as string
        String str;
        Error err = _decode_string(buf, len, r_len, str);
        if (err) {
            return err;
        }
        if (str.is_empty()) {
            r_variant = (Object *)nullptr;
        } else {
            String json_err;
            Json json = Json::parse(str, json_err);
            StringName class_name = json["typeName"].string_value();
            ERR_FAIL_COND_V(!ClassDB::can_instantiate(class_name), ERR_INVALID_DATA);
            ERR_FAIL_COND_V(!json_err.is_empty(), ERR_INVALID_DATA);
            // @todo 判断object与该class的继承关系
            void* instance = ClassDB::instantiate_with_json(json);
            ERR_FAIL_COND_V(!instance, ERR_INVALID_DATA);
            r_variant = (Object*) instance;
        }

    }
    }
    break;
    case Variant::CALLABLE:
    {
        // r_variant = Callable(); // @todo
    }
    break;
    case Variant::SIGNAL:
    {
        String name;
        Error err = _decode_string(buf, len, r_len, name);
        if (err)
        {
            return err;
        }

        ERR_FAIL_COND_V(len < 8, ERR_INVALID_DATA);
        ObjectID id = ObjectID(decode_uint64(buf));
        if (r_len)
        {
            (*r_len) += 8;
        }

        r_variant = Signal(id, StringName(name));
    }
    break;
    case Variant::DICTIONARY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        //  bool shared = count&0x80000000;
        count &= 0x7FFFFFFF;

        buf += 4;
        len -= 4;

        if (r_len)
        {
            (*r_len) += 4; // Size of count number.
        }

        Dictionary d;

        for (int i = 0; i < count; i++)
        {
            Variant key, value;

            int used;
            Error err = decode_variant(key, buf, len, &used, p_allow_objects, p_depth + 1);
            ERR_FAIL_COND_V_MSG(err != OK, err, "Error when trying to decode Variant.");

            buf += used;
            len -= used;
            if (r_len)
            {
                (*r_len) += used;
            }

            err = decode_variant(value, buf, len, &used, p_allow_objects, p_depth + 1);
            ERR_FAIL_COND_V_MSG(err != OK, err, "Error when trying to decode Variant.");

            buf += used;
            len -= used;
            if (r_len)
            {
                (*r_len) += used;
            }

            d[key] = value;
        }

        r_variant = d;
    }
    break;
    // case Variant::ARRAY:
    // {
    //     Variant::Type builtin_type = Variant::VARIANT_MAX;
    //     StringName class_name;
    //     Ref<Script> script;

    //     switch (header & HEADER_DATA_FIELD_TYPED_ARRAY_MASK)
    //     {
    //     case HEADER_DATA_FIELD_TYPED_ARRAY_NONE:
    //         break; // Untyped array.
    //     case HEADER_DATA_FIELD_TYPED_ARRAY_BUILTIN:
    //     {
    //         ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);

    //         int32_t bt = decode_uint32(buf);
    //         buf += 4;
    //         len -= 4;
    //         if (r_len)
    //         {
    //             (*r_len) += 4;
    //         }

    //         ERR_FAIL_INDEX_V(bt, Variant::VARIANT_MAX, ERR_INVALID_DATA);
    //         builtin_type = (Variant::Type)bt;
    //         if (!p_allow_objects && builtin_type == Variant::OBJECT)
    //         {
    //             class_name = EncodedObjectAsID::get_class_static();
    //         }
    //     }
    //     break;
    //     case HEADER_DATA_FIELD_TYPED_ARRAY_CLASS_NAME:
    //     {
    //         String str;
    //         Error err = _decode_string(buf, len, r_len, str);
    //         if (err)
    //         {
    //             return err;
    //         }

    //         builtin_type = Variant::OBJECT;
    //         if (p_allow_objects)
    //         {
    //             class_name = str;
    //         }
    //         else
    //         {
    //             class_name = EncodedObjectAsID::get_class_static();
    //         }
    //     }
    //     break;
    //     case HEADER_DATA_FIELD_TYPED_ARRAY_SCRIPT:
    //     {
    //         String path;
    //         Error err = _decode_string(buf, len, r_len, path);
    //         if (err)
    //         {
    //             return err;
    //         }

    //         builtin_type = Variant::OBJECT;
    //         if (p_allow_objects)
    //         {
    //             ERR_FAIL_COND_V_MSG(path.is_empty() || !path.begins_with("res://") || !ResourceLoader::exists(path, "Script"), ERR_INVALID_DATA, "Invalid script path: '" + path + "'.");
    //             script = ResourceLoader::load(path, "Script");
    //             ERR_FAIL_COND_V_MSG(script.is_null(), ERR_INVALID_DATA, "Can't load script at path: '" + path + "'.");
    //             class_name = script->get_instance_base_type();
    //         }
    //         else
    //         {
    //             class_name = EncodedObjectAsID::get_class_static();
    //         }
    //     }
    //     break;
    //     default:
    //         ERR_FAIL_V(ERR_INVALID_DATA); // Future proofing.
    //     }

    //     ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);

    //     int32_t count = decode_uint32(buf);
    //     //  bool shared = count&0x80000000;
    //     count &= 0x7FFFFFFF;

    //     buf += 4;
    //     len -= 4;

    //     if (r_len)
    //     {
    //         (*r_len) += 4; // Size of count number.
    //     }

    //     Array varr;
    //     if (builtin_type != Variant::VARIANT_MAX)
    //     {
    //         varr.set_typed(builtin_type, class_name, script);
    //     }

    //     for (int i = 0; i < count; i++)
    //     {
    //         int used = 0;
    //         Variant v;
    //         Error err = decode_variant(v, buf, len, &used, p_allow_objects, p_depth + 1);
    //         ERR_FAIL_COND_V_MSG(err != OK, err, "Error when trying to decode Variant.");
    //         buf += used;
    //         len -= used;
    //         varr.push_back(v);
    //         if (r_len)
    //         {
    //             (*r_len) += used;
    //         }
    //     }

    //     r_variant = varr;
    // }
    // break;

    // arrays
    case Variant::PACKED_BYTE_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;
        ERR_FAIL_COND_V(count < 0 || count > len, ERR_INVALID_DATA);

        Vector<uint8_t> data;

        if (count)
        {
            data.resize(count);
            uint8_t *w = data.ptrw();
            for (int32_t i = 0; i < count; i++)
            {
                w[i] = buf[i];
            }
        }

        r_variant = data;

        if (r_len)
        {
            if (count % 4)
            {
                (*r_len) += 4 - count % 4;
            }
            (*r_len) += 4 + count;
        }
    }
    break;
    case Variant::PACKED_INT32_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;
        ERR_FAIL_MUL_OF(count, 4, ERR_INVALID_DATA);
        ERR_FAIL_COND_V(count < 0 || count * 4 > len, ERR_INVALID_DATA);

        Vector<int32_t> data;

        if (count)
        {
            // const int*rbuf=(const int*)buf;
            data.resize(count);
            int32_t *w = data.ptrw();
            for (int32_t i = 0; i < count; i++)
            {
                w[i] = decode_uint32(&buf[i * 4]);
            }
        }
        r_variant = Variant(data);
        if (r_len)
        {
            (*r_len) += 4 + count * sizeof(int32_t);
        }
    }
    break;
    case Variant::PACKED_INT64_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;
        ERR_FAIL_MUL_OF(count, 8, ERR_INVALID_DATA);
        ERR_FAIL_COND_V(count < 0 || count * 8 > len, ERR_INVALID_DATA);

        Vector<int64_t> data;

        if (count)
        {
            // const int*rbuf=(const int*)buf;
            data.resize(count);
            int64_t *w = data.ptrw();
            for (int64_t i = 0; i < count; i++)
            {
                w[i] = decode_uint64(&buf[i * 8]);
            }
        }
        r_variant = Variant(data);
        if (r_len)
        {
            (*r_len) += 4 + count * sizeof(int64_t);
        }
    }
    break;
    case Variant::PACKED_FLOAT32_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;
        ERR_FAIL_MUL_OF(count, 4, ERR_INVALID_DATA);
        ERR_FAIL_COND_V(count < 0 || count * 4 > len, ERR_INVALID_DATA);

        Vector<float> data;

        if (count)
        {
            // const float*rbuf=(const float*)buf;
            data.resize(count);
            float *w = data.ptrw();
            for (int32_t i = 0; i < count; i++)
            {
                w[i] = decode_float(&buf[i * 4]);
            }
        }
        r_variant = data;

        if (r_len)
        {
            (*r_len) += 4 + count * sizeof(float);
        }
    }
    break;
    case Variant::PACKED_FLOAT64_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;
        ERR_FAIL_MUL_OF(count, 8, ERR_INVALID_DATA);
        ERR_FAIL_COND_V(count < 0 || count * 8 > len, ERR_INVALID_DATA);

        Vector<double> data;

        if (count)
        {
            data.resize(count);
            double *w = data.ptrw();
            for (int64_t i = 0; i < count; i++)
            {
                w[i] = decode_double(&buf[i * 8]);
            }
        }
        r_variant = data;

        if (r_len)
        {
            (*r_len) += 4 + count * sizeof(double);
        }
    }
    break;
    case Variant::PACKED_STRING_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);

        Vector<String> strings;
        buf += 4;
        len -= 4;

        if (r_len)
        {
            (*r_len) += 4; // Size of count number.
        }

        for (int32_t i = 0; i < count; i++)
        {
            String str;
            Error err = _decode_string(buf, len, r_len, str);
            if (err)
            {
                return err;
            }

            strings.push_back(str);
        }

        r_variant = strings;
    }
    break;
    case Variant::PACKED_VECTOR2_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;

        Vector<Vector2> varray;

        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_MUL_OF(count, sizeof(double) * 2, ERR_INVALID_DATA);
            ERR_FAIL_COND_V(count < 0 || count * sizeof(double) * 2 > (size_t)len, ERR_INVALID_DATA);

            if (r_len)
            {
                (*r_len) += 4; // Size of count number.
            }

            if (count)
            {
                varray.resize(count);
                Vector2 *w = varray.ptrw();

                for (int32_t i = 0; i < count; i++)
                {
                    w[i].x = decode_double(buf + i * sizeof(double) * 2 + sizeof(double) * 0);
                    w[i].y = decode_double(buf + i * sizeof(double) * 2 + sizeof(double) * 1);
                }

                int adv = sizeof(double) * 2 * count;

                if (r_len)
                {
                    (*r_len) += adv;
                }
                len -= adv;
                buf += adv;
            }
        }
        else
        {
            ERR_FAIL_MUL_OF(count, sizeof(float) * 2, ERR_INVALID_DATA);
            ERR_FAIL_COND_V(count < 0 || count * sizeof(float) * 2 > (size_t)len, ERR_INVALID_DATA);

            if (r_len)
            {
                (*r_len) += 4; // Size of count number.
            }

            if (count)
            {
                varray.resize(count);
                Vector2 *w = varray.ptrw();

                for (int32_t i = 0; i < count; i++)
                {
                    w[i].x = decode_float(buf + i * sizeof(float) * 2 + sizeof(float) * 0);
                    w[i].y = decode_float(buf + i * sizeof(float) * 2 + sizeof(float) * 1);
                }

                int adv = sizeof(float) * 2 * count;

                if (r_len)
                {
                    (*r_len) += adv;
                }
            }
        }
        r_variant = varray;
    }
    break;
    case Variant::PACKED_VECTOR3_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;

        Vector<Vector3> varray;

        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_MUL_OF(count, sizeof(double) * 3, ERR_INVALID_DATA);
            ERR_FAIL_COND_V(count < 0 || count * sizeof(double) * 3 > (size_t)len, ERR_INVALID_DATA);

            if (r_len)
            {
                (*r_len) += 4; // Size of count number.
            }

            if (count)
            {
                varray.resize(count);
                Vector3 *w = varray.ptrw();

                for (int32_t i = 0; i < count; i++)
                {
                    w[i].x = decode_double(buf + i * sizeof(double) * 3 + sizeof(double) * 0);
                    w[i].y = decode_double(buf + i * sizeof(double) * 3 + sizeof(double) * 1);
                    w[i].z = decode_double(buf + i * sizeof(double) * 3 + sizeof(double) * 2);
                }

                int adv = sizeof(double) * 3 * count;

                if (r_len)
                {
                    (*r_len) += adv;
                }
                len -= adv;
                buf += adv;
            }
        }
        else
        {
            ERR_FAIL_MUL_OF(count, sizeof(float) * 3, ERR_INVALID_DATA);
            ERR_FAIL_COND_V(count < 0 || count * sizeof(float) * 3 > (size_t)len, ERR_INVALID_DATA);

            if (r_len)
            {
                (*r_len) += 4; // Size of count number.
            }

            if (count)
            {
                varray.resize(count);
                Vector3 *w = varray.ptrw();

                for (int32_t i = 0; i < count; i++)
                {
                    w[i].x = decode_float(buf + i * sizeof(float) * 3 + sizeof(float) * 0);
                    w[i].y = decode_float(buf + i * sizeof(float) * 3 + sizeof(float) * 1);
                    w[i].z = decode_float(buf + i * sizeof(float) * 3 + sizeof(float) * 2);
                }

                int adv = sizeof(float) * 3 * count;

                if (r_len)
                {
                    (*r_len) += adv;
                }
                len -= adv;
                buf += adv;
            }
        }
        r_variant = varray;
    }
    break;
    case Variant::PACKED_COLOR_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;

        ERR_FAIL_MUL_OF(count, 4 * 4, ERR_INVALID_DATA);
        ERR_FAIL_COND_V(count < 0 || count * 4 * 4 > len, ERR_INVALID_DATA);

        Vector<Color> carray;

        if (r_len)
        {
            (*r_len) += 4; // Size of count number.
        }

        if (count)
        {
            carray.resize(count);
            Color *w = carray.ptrw();

            for (int32_t i = 0; i < count; i++)
            {
                // Colors should always be in single-precision.
                w[i].r = decode_float(buf + i * 4 * 4 + 4 * 0);
                w[i].g = decode_float(buf + i * 4 * 4 + 4 * 1);
                w[i].b = decode_float(buf + i * 4 * 4 + 4 * 2);
                w[i].a = decode_float(buf + i * 4 * 4 + 4 * 3);
            }

            int adv = 4 * 4 * count;

            if (r_len)
            {
                (*r_len) += adv;
            }
        }

        r_variant = carray;
    }
    break;

    case Variant::PACKED_VECTOR4_ARRAY:
    {
        ERR_FAIL_COND_V(len < 4, ERR_INVALID_DATA);
        int32_t count = decode_uint32(buf);
        buf += 4;
        len -= 4;

        Vector<Vector4> varray;

        if (header & HEADER_DATA_FLAG_64)
        {
            ERR_FAIL_MUL_OF(count, sizeof(double) * 4, ERR_INVALID_DATA);
            ERR_FAIL_COND_V(count < 0 || count * sizeof(double) * 4 > (size_t)len, ERR_INVALID_DATA);

            if (r_len)
            {
                (*r_len) += 4; // Size of count number.
            }

            if (count)
            {
                varray.resize(count);
                Vector4 *w = varray.ptrw();

                for (int32_t i = 0; i < count; i++)
                {
                    w[i].x = decode_double(buf + i * sizeof(double) * 4 + sizeof(double) * 0);
                    w[i].y = decode_double(buf + i * sizeof(double) * 4 + sizeof(double) * 1);
                    w[i].z = decode_double(buf + i * sizeof(double) * 4 + sizeof(double) * 2);
                    w[i].w = decode_double(buf + i * sizeof(double) * 4 + sizeof(double) * 3);
                }

                int adv = sizeof(double) * 4 * count;

                if (r_len)
                {
                    (*r_len) += adv;
                }
                len -= adv;
                buf += adv;
            }
        }
        else
        {
            ERR_FAIL_MUL_OF(count, sizeof(float) * 4, ERR_INVALID_DATA);
            ERR_FAIL_COND_V(count < 0 || count * sizeof(float) * 4 > (size_t)len, ERR_INVALID_DATA);

            if (r_len)
            {
                (*r_len) += 4; // Size of count number.
            }

            if (count)
            {
                varray.resize(count);
                Vector4 *w = varray.ptrw();

                for (int32_t i = 0; i < count; i++)
                {
                    w[i].x = decode_float(buf + i * sizeof(float) * 4 + sizeof(float) * 0);
                    w[i].y = decode_float(buf + i * sizeof(float) * 4 + sizeof(float) * 1);
                    w[i].z = decode_float(buf + i * sizeof(float) * 4 + sizeof(float) * 2);
                    w[i].w = decode_float(buf + i * sizeof(float) * 4 + sizeof(float) * 3);
                }

                int adv = sizeof(float) * 4 * count;

                if (r_len)
                {
                    (*r_len) += adv;
                }
                len -= adv;
                buf += adv;
            }
        }
        r_variant = varray;
    }
    break;
    default:
    {
        ERR_FAIL_V(ERR_BUG);
    }
    }

    return OK;
}
}