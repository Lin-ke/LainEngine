
#include "runtime/core/meta/meta_example.h"
#include "_generated/serializer/all_serializer.h"
#include "runtime/core/meta/serializer/serializer.h"
#include "runtime/core/log/log.h"
#include <filesystem>
#include <fstream>
#include <iostream>
namespace lain
{
    void metaExample()
    {
        Test1 test1_in;
        test1_in.m_int  = 12;
        test1_in.m_char = 'g';
        int i           = 1;
        test1_in.m_int_vector.emplace_back(&i);

        Test1 test1_out;
        // test on array
        Test2 test2_in;
        test2_in.m_test_base_array.emplace_back("Test1", &test1_in);
        Test1 Test2_temp;
        test2_in.m_test_base_array.emplace_back("Test1", &Test2_temp);

        // serializer & deserializer

        // write Test1_in (object) to Test1_json_in (json)
        auto test1_json_in = Serializer::write(test1_in);

        std::string test1_context = test1_json_in.dump();
        L_CORE_INFO(test1_context);
        // read Test1_context (json) to Test1_out (object)
        std::string err;

        auto&& Test1_json = Json::parse(test1_context, err);
        Serializer::read(Test1_json, test1_out);
        //L_CORE_INFO(test1_context);

        auto        Test2_json_in = Serializer::write(test2_in);
        std::string test2_context = Test2_json_in.dump();

        std::fstream out_put("out.txt", std::ios::out);
        out_put << test2_context;
        out_put.flush();
        out_put.close();

        Test2  test2_out;
        auto&& test2_json = Json::parse(test2_context, err);
        Serializer::read(test2_json, test2_out);
        //L_CORE_INFO(test2_context.c_str());


        // reflection
        // get meta from TypeMetaDef, returns : reflection
        auto                       meta = TypeMetaDef(Test2, &test2_out);
        Reflection::FieldAccessor* fields;

        int                        fields_count = meta.m_meta.getFieldsList(fields);
        std::cout << "fields_count:" << fields_count << std::endl;
        for (int i = 0; i < fields_count; ++i)
        {
            auto filed_accesser = fields[i];
            std::cout << filed_accesser.getFieldTypeName() << " " << filed_accesser.getFieldName() << " "  << std::endl;
            if (filed_accesser.isArrayType())
            {
                Reflection::ArrayAccessor array_accesser;
                if (Reflection::TypeMeta::newArrayAccessorFromName(filed_accesser.getFieldTypeName(), array_accesser))
                {
                    void* field_instance = filed_accesser.get(meta.m_instance);
                    int   count          = array_accesser.getSize(field_instance);
                    auto  typeMetaItem   = Reflection::TypeMeta::newMetaFromName(array_accesser.getElementTypeName());
                    for (int index = 0; index < count; ++index)
                    {
                        std::cout << ":L:" << index << ":R:" << (int*)array_accesser.get(index, field_instance)
                                  << std::endl;
                        ;
                    }
                }
            } 
            else {

            }
        }
    }
} // namespace lain
