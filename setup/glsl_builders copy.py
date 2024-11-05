"""Functions used to generate source files during build time"""

import os.path
from typing import Iterable, Optional

from methods import print_error


def generate_inline_code(input_lines: Iterable[str], insert_newline: bool = True):
    """Take header data and generate inline code

    :param: input_lines: values for shared inline code
    :return: str - generated inline value
    """
    output = []
    for line in input_lines:
        if line:
            output.append(",".join(str(ord(c)) for c in line))
        if insert_newline:
            output.append("%s" % ord("\n"))
    output.append("0")
    return ",".join(output)


class RDHeaderStruct:
    def __init__(self):
        self.vertex_lines = []
        self.fragment_lines = []
        self.compute_lines = []

        self.vertex_included_files = []
        self.fragment_included_files = []
        self.compute_included_files = []

        self.reading = ""
        self.line_offset = 0
        self.vertex_offset = 0
        self.fragment_offset = 0
        self.compute_offset = 0


def include_file_in_rd_header(filename: str, header_data: RDHeaderStruct, depth: int) -> RDHeaderStruct:
    with open(filename, "r", encoding="utf-8") as fs:
        line = fs.readline()

        while line:
            index = line.find("//")
            if index != -1:
                line = line[:index]

            if line.find("#[vertex]") != -1:
                header_data.reading = "vertex"
                line = fs.readline()
                header_data.line_offset += 1
                header_data.vertex_offset = header_data.line_offset
                continue

            if line.find("#[fragment]") != -1:
                header_data.reading = "fragment"
                line = fs.readline()
                header_data.line_offset += 1
                header_data.fragment_offset = header_data.line_offset
                continue

            if line.find("#[compute]") != -1:
                header_data.reading = "compute"
                line = fs.readline()
                header_data.line_offset += 1
                header_data.compute_offset = header_data.line_offset
                continue

            while line.find("#include ") != -1:
                includeline = line.replace("#include ", "").strip()[1:-1]

                if includeline.startswith("thirdparty/"):
                    included_file = os.path.relpath(includeline)

                else:
                    included_file = os.path.relpath(os.path.dirname(filename) + "/" + includeline)

                if included_file not in header_data.vertex_included_files and header_data.reading == "vertex":
                    header_data.vertex_included_files += [included_file]
                    if include_file_in_rd_header(included_file, header_data, depth + 1) is None:
                        print_error(f'In file "{filename}": #include "{includeline}" could not be found!"')
                elif included_file not in header_data.fragment_included_files and header_data.reading == "fragment":
                    header_data.fragment_included_files += [included_file]
                    if include_file_in_rd_header(included_file, header_data, depth + 1) is None:
                        print_error(f'In file "{filename}": #include "{includeline}" could not be found!"')
                elif included_file not in header_data.compute_included_files and header_data.reading == "compute":
                    header_data.compute_included_files += [included_file]
                    if include_file_in_rd_header(included_file, header_data, depth + 1) is None:
                        print_error(f'In file "{filename}": #include "{includeline}" could not be found!"')

                line = fs.readline()

            line = line.replace("\r", "").replace("\n", "")

            if header_data.reading == "vertex":
                header_data.vertex_lines += [line]
            if header_data.reading == "fragment":
                header_data.fragment_lines += [line]
            if header_data.reading == "compute":
                header_data.compute_lines += [line]

            line = fs.readline()
            header_data.line_offset += 1

    return header_data


def build_rd_header(
    filename: str, optional_output_filename: Optional[str] = None, header_data: Optional[RDHeaderStruct] = None
) -> None:
    header_data = header_data or RDHeaderStruct()
    include_file_in_rd_header(filename, header_data, 0)

    if optional_output_filename is None:
        out_file = filename + ".gen.h"
    else:
        out_file = optional_output_filename

    out_file_base = out_file
    out_file_base = out_file_base[out_file_base.rfind("/") + 1 :] # 找到最后一个/的位置，然后取后面的字符串
    out_file_base = out_file_base[out_file_base.rfind("\\") + 1 :] # 找到最后一个\的位置，然后取后面的字符串
    out_file_ifdef = out_file_base.replace(".", "_").upper()
    out_file_class = out_file_base.replace(".glsl.gen.h", "").title().replace("_", "").replace(".", "") + "ShaderRD" # 将文件名转换为类名
    
    if header_data.compute_lines:
        compute_code_str = generate_inline_code(header_data.compute_lines)
        compute_shader_file = out_file_class + ".compute.lglsl"
        # 保存
        with open(compute_shader_file, "w", encoding="utf-8", newline="\n") as f:
            f.write(compute_code_str)
        body_parts = [
            "Error err;",
            "Ref<FileAccess> file = FileAccess::open(\"%s\", FileAccess::READ,  &err);" % compute_shader_file,
            "ERR_FAIL_COND_V_MSG(err != OK, err, \"Open glsl file error.\");",
            "static const String _compute_code = file->get_as_utf8_string();",
            f'setup(String(), String(), _compute_code, "{out_file_class}");',
        ]
    else:
        vertex_code_str = generate_inline_code(header_data.vertex_lines)
        fragment_code_str = generate_inline_code(header_data.fragment_lines)
        vertex_shader_file = out_file_class + ".vertex.lglsl"
        fragment_shader_file = out_file_class + ".fragment.lglsl"
        # 保存
        with open(vertex_shader_file, "w", encoding="utf-8", newline="\n") as f:
            f.write(vertex_code_str)
        with open(fragment_shader_file, "w", encoding="utf-8", newline="\n") as f:
            f.write(fragment_code_str)
        body_parts = [
            "Error err;",
            "Ref<FileAccess> file1 = FileAccess::open(\"%s\", FileAccess::READ,  &err);" % vertex_shader_file,
            "ERR_FAIL_COND_V_MSG(err != OK, err, \"Open glsl file error.\");",
            "static const String _vertex_code = file1->get_as_utf8_string();",
            "Ref<FileAccess> file2 = FileAccess::open(\"%s\", FileAccess::READ,  &err);" % fragment_shader_file,
            "ERR_FAIL_COND_V_MSG(err != OK, err, \"Read glsl file error.\");",
            "static const String _fragment_code = file2->get_as_utf8_string();",
            f'setup(_vertex_code, _fragment_code, String(), "{out_file_class}");',
        ]

    body_content = "\n\t\t".join(body_parts)

    # Intended curly brackets are doubled so f-string doesn't eat them up.
    shader_template = f"""/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef {out_file_ifdef}_RD
#define {out_file_ifdef}_RD

#include "servers/rendering/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"

class {out_file_class} : public ShaderRD {{

public:

	{out_file_class}() {{

		{body_content}
	}}
}};

#endif
"""

    with open(out_file, "w", encoding="utf-8", newline="\n") as fd:
        fd.write(shader_template)


def build_rd_headers(target, source, env):
    for x in source:
        build_rd_header(filename=str(x))


class RAWHeaderStruct:
    def __init__(self):
        self.code = ""


def include_file_in_raw_header(filename: str, header_data: RAWHeaderStruct, depth: int) -> None:
    with open(filename, "r", encoding="utf-8") as fs:
        line = fs.readline()

        while line:
            while line.find("#include ") != -1:
                includeline = line.replace("#include ", "").strip()[1:-1]

                included_file = os.path.relpath(os.path.dirname(filename) + "/" + includeline)
                include_file_in_raw_header(included_file, header_data, depth + 1)

                line = fs.readline()

            header_data.code += line
            line = fs.readline()


def build_raw_header(
    filename: str, optional_output_filename: Optional[str] = None, header_data: Optional[RAWHeaderStruct] = None
):
    header_data = header_data or RAWHeaderStruct()
    include_file_in_raw_header(filename, header_data, 0)

    if optional_output_filename is None:
        out_file = filename + ".gen.h"
    else:
        out_file = optional_output_filename

    out_file_base = out_file.replace(".glsl.gen.h", "_shader_glsl")
    out_file_base = out_file_base[out_file_base.rfind("/") + 1 :]
    out_file_base = out_file_base[out_file_base.rfind("\\") + 1 :]
    out_file_ifdef = out_file_base.replace(".", "_").upper()

    shader_template = f"""/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef {out_file_ifdef}_RAW_H
#define {out_file_ifdef}_RAW_H

static const char {out_file_base}[] = {{
    {generate_inline_code(header_data.code, insert_newline=False)}
}};
#endif
"""

    with open(out_file, "w", encoding="utf-8", newline="\n") as f:
        f.write(shader_template)


def build_raw_headers(target, source, env):
    for x in source:
        build_raw_header(filename=str(x))
