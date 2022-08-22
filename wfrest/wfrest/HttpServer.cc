#include "workflow/HttpMessage.h"

#include <utility>

#include "wfrest/HttpServer.h"
#include "wfrest/HttpServerTask.h"
#include "wfrest/UriUtil.h"
#include "wfrest/HttpFile.h"
#include "wfrest/PathUtil.h"
#include "wfrest/Macro.h"
#include "wfrest/Router.h"
#include "wfrest/json.hpp"
#include "wfrest/ErrorCode.h"
#include "XLogger.h"

using namespace wfrest;
using Json = nlohmann::json;

void HttpServer::process(HttpTask *task)
{
    auto *server_task = static_cast<HttpServerTask *>(task);

    auto *req = server_task->get_req();
    auto *resp = server_task->get_resp();
    long long seq = server_task->get_task_seq();
    req->fill_header_map();
    req->fill_content_type();
	char addrstr[128];
	struct sockaddr_storage addr;
	socklen_t l = sizeof addr;
	unsigned short port = 0;
	server_task->get_peer_addr((struct sockaddr*)&addr, &l);
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in* sin = (struct sockaddr_in*)&addr;
		inet_ntop(AF_INET, &sin->sin_addr, addrstr, 128);
		port = ntohs(sin->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, 128);
		port = ntohs(sin6->sin6_port);
	}
    else
    {
        strcpy(addrstr, "Unknown Client address");
    }
    XLOG_INFO("Peer address:{:s}:{:d},seq:{:d}", addrstr, port,seq);

    resp->headers["Server"] = serverName;
    resp->headers["Access-Control-Allow-Origin"] = "*";

    size_t maxSeq = this->params.max_connections / 10;
	if (seq == maxSeq) /* no more than 10 requests on the same connection. */
    {
        std::string msg=StrUtil::format("no more than %d requests on the same connection", maxSeq);
        XLOG_ERROR(msg);
		//resp->set_http_version("HTTP/1.1");
		//resp->set_status_code("200");
		//resp->set_reason_phrase("OK");
		//resp->add_header_pair("Content-Type", "application/json;charset=UTF-8");
        //resp->add_header_pair("Connection", "close");
        Json json;
        json["code"] = HttpStatusOK;
        json["msg"] = msg;
        std::string res = json.dump();
        resp->append_output_body(res.c_str(), res.length());
        resp->headers["Content-Type"] = "application/json;charset=UTF-8"; //必须使用resp的heades增加头，如果使用了add_header_pair会冲突
        resp->headers["Connection"] = "close";
        return;
    }
		
    const std::string &host = req->header("Host");
    
    if (host.empty())
    {
        std::string msg = "header Host not found";
        //header Host not found
        XLOG_ERROR(msg);
        resp->set_status(HttpStatusBadRequest);
        Json json;
        json["code"] = HttpStatusBadRequest;
        json["msg"] = msg;
        std::string res = json.dump();
        resp->append_output_body(res.c_str(), res.length());
        resp->headers["Content-Type"] = "application/json;charset=UTF-8"; //必须使用resp的heades增加头，如果使用了add_header_pair会冲突
        return;
    }
    
    std::string request_uri = "http://" + host + req->get_request_uri();  // or can't parse URI
    ParsedURI uri;
    if (URIParser::parse(request_uri, uri) < 0)
    {
        //resp->set_status(HttpStatusBadRequest);
        std::string msg = "parse uri error";
        XLOG_ERROR(msg);
        resp->set_status(HttpStatusBadRequest);
        Json json;
        json["code"] = HttpStatusBadRequest;
        json["msg"] = msg;
        std::string res = json.dump();
        resp->append_output_body(res.c_str(), res.length());
        resp->headers["Content-Type"] = "application/json;charset=UTF-8";
        return;
    }

    std::string route;    
    if (uri.path && uri.path[0])
        route = uri.path;
    else
        route = "/";

    if (uri.query)
    {
        StringPiece query(uri.query);
        req->set_query_params(UriUtil::split_query(query));
    }
    
    if (route.back() == '/')
        route += "index.html";
    req->set_parsed_uri(std::move(uri));
	std::string verb = req->get_method();
	XLOG_INFO("method:{:s},url:{:s}", verb, route);
    int ret = blue_print_.router().call(str_to_verb(verb), route, server_task);//查找请求是否已注册
    if(ret != StatusOK)
    {
        resp->Error(ret, verb + " " + route);
    }
    if(track_func_)
    {
        server_task->add_callback(track_func_);
    }
}

CommSession *HttpServer::new_session(long long seq, CommConnection *conn)
{
    HttpTask *task = new HttpServerTask(this, this->WFServer<HttpReq, HttpResp>::process);
    task->set_keep_alive(this->params.keep_alive_timeout);
    task->set_receive_timeout(this->params.receive_timeout);
    task->get_req()->set_size_limit(this->params.request_size_limit);

    return task;
}

void HttpServer::list_routes()
{
    blue_print_.router().print_routes();
}

void HttpServer::register_blueprint(const BluePrint& bp, const std::string& url_prefix)
{
    blue_print_.add_blueprint(bp, url_prefix);
}

// /static : /www/file/
void HttpServer::Static(const char *relative_path, const char *root)
{
    BluePrint bp;
    int ret = serve_static(root, OUT bp);
    if(ret != StatusOK)
    {
        XLOG_ERROR("Error:{} dose not exists",root);
        return;
    }
    blue_print_.add_blueprint(std::move(bp), relative_path);
}

int HttpServer::serve_static(const char* path, OUT BluePrint &bp)
{
    std::string path_str(path);
    bool is_file = true;
    if (PathUtil::is_dir(path_str))
    {
        is_file = false;
    } else if(!PathUtil::is_file(path_str))
    {
        return StatusNotFound;
    }    
    bp.GET("/*", [path_str, is_file](const HttpReq *req, HttpResp *resp) {
        std::string match_path = req->match_path();
        if(is_file && match_path.empty())
        {
            resp->File(path_str);
        } else 
        {
            resp->File(path_str + "/" + match_path);
        }
    });
    return StatusOK;
}

HttpServer &HttpServer::track()
{
    track_func_ = [](HttpTask *server_task) {
        HttpResp *resp = server_task->get_resp();
        HttpReq *req = server_task->get_req();
        HttpServerTask *task = static_cast<HttpServerTask *>(server_task);
        Timestamp current_time = Timestamp::now();
        std::string fmt_time = current_time.to_format_str();
        XLOG_INFO("{} | {} | {} | {} | \"{}\" | --",
                    fmt_time.c_str(),
                    resp->get_status_code(),
                    task->get_peer_addr_str().c_str(),
                    req->get_method(),
                    req->current_path().c_str());
    };
    return *this;
}

HttpServer &HttpServer::track(const TrackFunc &track_func)
{
    track_func_ = track_func;
    return *this;
}

HttpServer &HttpServer::track(TrackFunc &&track_func)
{
    track_func_ = std::move(track_func);
    return *this;
}
