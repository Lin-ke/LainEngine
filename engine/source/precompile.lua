function precompile()
    local p = string.sub(path.getabsolute("runtime"), 1 ,-8)
    Runtime_headers = os.matchfiles("runtime/**.h")
    -- 不要再这里写editor的，
    All_headers = {}
    for _, v in ipairs(Runtime_headers) do
        table.insert(All_headers, ( p .. v))
    end
    
    json_path = "../precompile.json"
    save_file = io.open(json_path, "w")
    for k,v in ipairs(All_headers) do
                save_file:write( v .. ";")
    end
    json_path = path.getabsolute(json_path)
    return All_headers
end
precompile()
print("--Generate precompile.json--")
