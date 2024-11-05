-- Functions used to generate source files during build time

local function print_error(message)
    print("Error: " .. message)
end

local function generate_inline_code(input_lines, insert_newline)
    local output = {}
    for _, line in ipairs(input_lines) do
        if line ~= "" then
            local char_codes = {}
            for i = 1, #line do
                table.insert(char_codes, string.byte(line, i))
            end
            table.insert(output, table.concat(char_codes, ","))
        end
        if insert_newline then
            table.insert(output, string.byte("\n"))
        end
    end
    table.insert(output, "0")
    return table.concat(output, ",")
end

local RDHeaderStruct = {}
RDHeaderStruct.__index = RDHeaderStruct

function RDHeaderStruct:new()
    local self = setmetatable({}, RDHeaderStruct)
    self.vertex_lines = {}
    self.fragment_lines = {}
    self.compute_lines = {}
    self.vertex_included_files = {}
    self.fragment_included_files = {}
    self.compute_included_files = {}
    self.reading = ""
    self.line_offset = 0
    self.vertex_offset = 0
    self.fragment_offset = 0
    self.compute_offset = 0
    return self
end

local function include_file_in_rd_header(filename, header_data, depth)
    local fs = io.open(filename, "r")
    local line = fs:read("*line")

    while line do
        local index = string.find(line, "//")
        if index then
            line = string.sub(line, 1, index - 1)
        end

        if string.find(line, "#[vertex]") then
            header_data.reading = "vertex"
            line = fs:read("*line")
            header_data.line_offset = header_data.line_offset + 1
            header_data.vertex_offset = header_data.line_offset
        elseif string.find(line, "#[fragment]") then
            header_data.reading = "fragment"
            line = fs:read("*line")
            header_data.line_offset = header_data.line_offset + 1
            header_data.fragment_offset = header_data.line_offset
        elseif string.find(line, "#[compute]") then
            header_data.reading = "compute"
            line = fs:read("*line")
            header_data.line_offset = header_data.line_offset + 1
            header_data.compute_offset = header_data.line_offset
        end

        while string.find(line, "#include ") do
            local includeline = string.match(line, "#include \"(.-)\"")
            local included_file

            if string.find(includeline, "thirdparty/") then
                included_file = os.curdir() .. "/" .. includeline
            else
                included_file = os.curdir() .. "/" .. string.match(filename, "(.*/)") .. includeline
            end

            if header_data.reading == "vertex" and not table.contains(header_data.vertex_included_files, included_file) then
                table.insert(header_data.vertex_included_files, included_file)
                if not include_file_in_rd_header(included_file, header_data, depth + 1) then
                    print_error('In file "' .. filename .. '": #include "' .. includeline .. '" could not be found!')
                end
            elseif header_data.reading == "fragment" and not table.contains(header_data.fragment_included_files, included_file) then
                table.insert(header_data.fragment_included_files, included_file)
                if not include_file_in_rd_header(included_file, header_data, depth + 1) then
                    print_error('In file "' .. filename .. '": #include "' .. includeline .. '" could not be found!')
                end
            elseif header_data.reading == "compute" and not table.contains(header_data.compute_included_files, included_file) then
                table.insert(header_data.compute_included_files, included_file)
                if not include_file_in_rd_header(included_file, header_data, depth + 1) then
                    print_error('In file "' .. filename .. '": #include "' .. includeline .. '" could not be found!')
                end
            end

            line = fs:read("*line")
        end

        line = string.gsub(line, "\r", "").gsub(line, "\n", "")

        if header_data.reading == "vertex" then
            table.insert(header_data.vertex_lines, line)
        elseif header_data.reading == "fragment" then
            table.insert(header_data.fragment_lines, line)
        elseif header_data.reading == "compute" then
            table.insert(header_data.compute_lines, line)
        end

        line = fs:read("*line")
        header_data.line_offset = header_data.line_offset + 1
    end

    fs:close()
    return header_data
end

local function build_rd_header(filename, optional_output_filename, header_data)
    header_data = header_data or RDHeaderStruct:new()
    include_file_in_rd_header(filename, header_data, 0)

    local out_file = optional_output_filename or filename .. ".gen.h"
    local out_file_base = string.match(out_file, "[^/\\]+$")
    out_file_base = string.gsub(out_file_base, "%.glsl%.gen%.h$", "")
    local out_file_ifdef = string.upper(string.gsub(out_file_base, "%.", "_"))
    local out_file_class = string.gsub(out_file_base, "%.", "") .. "ShaderRD"

    local body_parts = {}

    if #header_data.compute_lines > 0 then
        local compute_code_str = generate_inline_code(header_data.compute_lines)
        local compute_shader_file = out_file_class .. ".compute.lglsl"
        local f = io.open(compute_shader_file, "w")
        f:write(compute_code_str)
        f:close()

        table.insert(body_parts, "Error err;")
        table.insert(body_parts, string.format('Ref<FileAccess> file = FileAccess::open("%s", FileAccess::READ,  &err);', compute_shader_file))
        table.insert(body_parts, 'ERR_FAIL_COND_V_MSG(err != OK, err, "Open glsl file error.");')
        table.insert(body_parts, 'static const String _compute_code = file->get_as_utf8_string();')
        table.insert(body_parts, string.format('setup(String(), String(), _compute_code, "%s");', out_file_class))
    else
        local vertex_code_str = generate_inline_code(header_data.vertex_lines)
        local fragment_code_str = generate_inline_code(header_data.fragment_lines)
        local vertex_shader_file = out_file_class .. ".vertex.lglsl"
        local fragment_shader_file = out_file_class .. ".fragment.lglsl"

        local f = io.open(vertex_shader_file, "w")
        f:write(vertex_code_str)
        f:close()

        f = io.open(fragment_shader_file, "w")
        f:write(fragment_code_str)
        f:close()

        table.insert(body_parts, "Error err;")
        table.insert(body_parts, string.format('Ref<FileAccess> file1 = FileAccess::open("%s", FileAccess::READ,  &err);', vertex_shader_file))
        table.insert(body_parts, 'ERR_FAIL_COND_V_MSG(err != OK, err, "Open glsl file error.");')
        table.insert(body_parts, 'static const String _vertex_code = file1->get_as_utf8_string();')
        table.insert(body_parts, string.format('Ref<FileAccess> file2 = FileAccess::open("%s", FileAccess::READ,  &err);', fragment_shader_file))
        table.insert(body_parts, 'ERR_FAIL_COND_V_MSG(err != OK, err, "Read glsl file error.");')
        table.insert(body_parts, 'static const String _fragment_code = file2->get_as_utf8_string();')
        table.insert(body_parts, string.format('setup(_vertex_code, _fragment_code, String(), "%s");', out_file_class))
    end

    local body_content = table.concat(body_parts, "\n\t\t")

    local shader_template = string.format([[
/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef %s_RD
#define %s_RD

#include "servers/rendering/renderer_rd/shader_rd.h"
#include "core/io/file_access.h"

class %s : public ShaderRD {

public:

	%s() {

		%s
	}
};

#endif
]], out_file_ifdef, out_file_ifdef, out_file_class, out_file_class, body_content)

    local fd = io.open(out_file, "w")
    fd:write(shader_template)
    fd:close()
end

function build_rd_headers(target, source, env)
    print(123)
    for _, x in ipairs(source) do
        print(x)
        build_rd_header(x)
    end
end

local RAWHeaderStruct = {}
RAWHeaderStruct.__index = RAWHeaderStruct

function RAWHeaderStruct:new()
    local self = setmetatable({}, RAWHeaderStruct)
    self.code = ""
    return self
end

local function include_file_in_raw_header(filename, header_data, depth)
    local fs = io.open(filename, "r")
    local line = fs:read("*line")

    while line do
        while string.find(line, "#include ") do
            local includeline = string.match(line, "#include \"(.-)\"")
            local included_file = os.curdir() .. "/" .. string.match(filename, "(.*/)") .. includeline
            include_file_in_raw_header(included_file, header_data, depth + 1)
            line = fs:read("*line")
        end
        header_data.code = header_data.code .. line
        line = fs:read("*line")
    end

    fs:close()
end

local function build_raw_header(filename, optional_output_filename, header_data)
    header_data = header_data or RAWHeaderStruct:new()
    include_file_in_raw_header(filename, header_data, 0)

    local out_file = optional_output_filename or filename .. ".gen.h"
    local out_file_base = string.match(out_file, "[^/\\]+$")
    out_file_base = string.gsub(out_file_base, "%.glsl%.gen%.h$", "_shader_glsl")
    local out_file_ifdef = string.upper(string.gsub(out_file_base, "%.", "_"))

    local shader_template = string.format([[
/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */
#ifndef %s_RAW_H
#define %s_RAW_H

static const char %s[] = {
    %s
};
#endif
]], out_file_ifdef, out_file_ifdef, out_file_base, generate_inline_code(header_data.code, false))

    local f = io.open(out_file, "w")
    f:write(shader_template)
    f:close()
end

local function build_raw_headers(target, source, env)
    for _, x in ipairs(source) do
        build_raw_header(x)
    end
end


