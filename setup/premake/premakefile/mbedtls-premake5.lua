project "mbedtls"
	language    "C"
	kind        "StaticLib"
	warnings    "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	includedirs { 'include' }

	if not _OPTIONS["no-zlib"] then
		defines     { 'MBEDTLS_ZLIB_SUPPORT' }
		includedirs { '../zlib' }
	end

	files
	{
		"include/**.h",
		"library/*.c"
	}


