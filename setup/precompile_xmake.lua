function precompile()
    local p = string.sub(os.projectdir() .. ("/engine/source/runtime"), 1 ,-8)
    Runtime_headers = os.files("$(projectdir)/engine/source/runtime/**.h")
    -- 不要再这里写editor的，
    All_headers = {}
    for _, v in ipairs(Runtime_headers) do
        table.insert(All_headers, v)
    end
    
    json_path = "$(projectdir)/engine/source/precompile.json"
    local save_file = io.open(json_path, "w")
    for k,v in ipairs(All_headers) do
        save_file:write( v .. ";")
    end
    save_file:close()
    io.replace(json_path, "\\", "/", { plain = true, encoding = "UTF-8" })
    return All_headers
end
function get_json_path()
    json_path = "$(projectdir)/engine/source/precompile.json"
    -- return path.translate(json_path)
    return os.projectdir() .. ("/engine/source/precompile.json")
end

function get_parser_order()
    -- local parser_input = path.translate("$(projectdir)/engine/source/parser_header.h")
    local parser_input = (os.projectdir()) .. ("/engine/source/parser_header.h")
    local json_path = get_json_path()
    -- local p = path.translate("$(projectdir)/engine/source")
    local p = (os.projectdir()) .. ("/engine/source")
    p = p.replace(p, "\\", "/", { plain = true, encoding = "UTF-8" })
    json_path = json_path.replace(json_path, "\\", "/", { plain = true, encoding = "UTF-8" })
    parser_input = parser_input.replace(parser_input, "\\", "/", { plain = true, encoding = "UTF-8" })
    local sysinclude = "*"
    return  json_path .. " " .. parser_input .. " " .. p .. " " .. sysinclude .. " lain 0"

end

function main()
    precompile()
    print("--Generate precompile.json--")
end
