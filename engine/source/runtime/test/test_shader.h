#include "function/shader/shader_preprocessor.h"
namespace lain::test{
  void test_shader(){
    String p_code = "#define a 123\nint b = a;";
    using namespace shader;
    ShaderPreprocessor processor;
    String path = "res://";
		String pp_code;
		HashSet<Ref<ShaderInclude>> new_dependencies;
    Error result = processor.preprocess(p_code, path, pp_code, nullptr, nullptr, nullptr, &new_dependencies);
    if(result == OK){
      L_PRINT(pp_code);
    }
    else{
      L_PRINT("Error");
    }
  }
}