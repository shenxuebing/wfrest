set(SRCS
        StrUtil.cc
        HttpMsg.cc
        Router.cc
        RouteTable.cc
        UriUtil.cc
        HttpDef.cc
        HttpContent.cc
        MultiPartParser.c
        HttpFile.cc
        PathUtil.cc
        HttpServerTask.cc
        HttpServer.cc
        Logger.cc
        LogStream.cc
        Timestamp.cc
        SysInfo.cc
        AsyncFileLogger.cc
        Compress.cc
        base64.cc
        BluePrint.cc
        FileUtil.cc
        HttpCookie.cc
        )

add_library(wfrest ${SRCS})
# add_dependencies(wfrest workflow)

include_directories(E:/vs2019workspace/workflow-windows/_include)
add_definitions (-D WIN32_LEAN_AND_MEAN)
add_definitions (-D OS_WINDOWS=1)
add_definitions (-D strdup=_strdup)
add_definitions (-D strcasecmp=_stricmp)
add_definitions (-D strncasecmp=_strnicmp)


target_link_libraries(wfrest workflow OpenSSL::SSL OpenSSL::Crypto pthread rt ZLIB::ZLIB)
if (Brotli_FOUND)
    target_link_libraries(wfrest Brotli_lib)
endif ()

install(TARGETS wfrest DESTINATION lib)

# file(GLOB HEADERS "*.h")
set(HEADERS
        Compress.h
        HttpMsg.h
        HttpServer.h
        Logger.h
        StringPiece.h
        PathUtil.h
        base64.h
        FileUtil.h
        Aspect.h
  )

install(FILES ${HEADERS} DESTINATION include/wfrest)

#ifdef OS_WINDOWS
#undef ERROR
#endif
