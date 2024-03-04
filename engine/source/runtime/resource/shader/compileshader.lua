function compilesinglefile(filename)
    if os.ishost("windows") then
        os.execute( "glslangValidator.exe -V "  .. filename .. " >> error.log")
    elseif os.ishost("linux") then
        os.execute( "glslangValidator -V "  .. filename .. " >> error.log")
    end
    result = io.popen( "type error.log")
    output = result:read('*a')
    result:close()
    failed = string.find(output, "error")
    if failed then
        print(output)
    end
    return output

    
end
function compileshaderfiles()
    
    p = path.getabsolute("./")
    vert = os.matchfiles("**.vert")
    frag = os.matchfiles("**.frag")
    for _, v in ipairs(vert) do
        compilesinglefile(p.."/"..v)
    end
    for _, v in ipairs(frag) do
        compilesinglefile(p.."/"..v)
    end
end
print("--Compile ShaderFiles--")
os.remove("error.log")
compileshaderfiles()
print("--END Compile ShaderFiles--")

