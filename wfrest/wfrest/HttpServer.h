﻿#ifndef WFREST_HTTPSERVER_H_
#define WFREST_HTTPSERVER_H_

#include "workflow/WFHttpServer.h"
#include "workflow/HttpUtil.h"

#include <unordered_map>
#include <string>

#include "wfrest/HttpMsg.h"
#include "wfrest/BluePrint.h"

namespace wfrest
{

class HttpServer : public WFServer<HttpReq, HttpResp>, public Noncopyable
{
public:
    // reserve basic interface
    void ROUTE(const char *route, const Handler &handler, Verb verb) 
    {
        blue_print_.ROUTE(route, handler, verb);
    }

    void ROUTE(const char *route, int compute_queue_id, const Handler &handler, Verb verb) 
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, verb);   
    }

    void ROUTE(const char *route, const Handler &handler, const std::vector<std::string> &methods)
    {
        blue_print_.ROUTE(route, handler, methods);
    }

    void ROUTE(const char *route, int compute_queue_id, 
                const Handler &handler, const std::vector<std::string> &methods)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, methods);
    }

    void GET(const char *route, const Handler &handler)
    {
        blue_print_.GET(route, handler);
    }

    void GET(const char *route, int compute_queue_id, const Handler &handler)
    {
        blue_print_.GET(route, compute_queue_id, handler);
    }

    void POST(const char *route, const Handler &handler)
    {
        blue_print_.POST(route, handler);
    }

    void POST(const char *route, int compute_queue_id, const Handler &handler) 
    {
        blue_print_.POST(route, compute_queue_id, handler);
    }

    void DELETE(const char *route, const Handler &handler) 
    {
        blue_print_.DELETE(route, handler);
    }

    void DELETE(const char *route, int compute_queue_id, const Handler &handler)
    {
        blue_print_.DELETE(route, compute_queue_id, handler);
    }

    void PATCH(const char *route, const Handler &handler)
    {
        blue_print_.PATCH(route, handler);
    }

    void PATCH(const char *route, int compute_queue_id, const Handler &handler)
    {
        blue_print_.PATCH(route, compute_queue_id, handler);
    }

    void PUT(const char *route, const Handler &handler)
    {
        blue_print_.PUT(route, handler);   
    }

    void PUT(const char *route, int compute_queue_id, const Handler &handler)
    {
        blue_print_.PUT(route, compute_queue_id, handler); 
    }
    
    void HEAD(const char *route, const Handler &handler)
    {
        blue_print_.HEAD(route, handler);
    }

    void HEAD(const char *route, int compute_queue_id, const Handler &handler)
    {
        blue_print_.HEAD(route, compute_queue_id, handler);
    }


public:
    template<typename... AP>
    void ROUTE(const char *route, const Handler &handler, 
                Verb verb, const AP &... ap)
    {
        blue_print_.ROUTE(route, handler, verb, ap...);
    }

    template<typename... AP>
    void ROUTE(const char *route, int compute_queue_id, 
                const Handler &handler, Verb verb, const AP &... ap)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, verb, ap...);
    }

    template<typename... AP>
    void ROUTE(const char *route, const Handler &handler, 
                const std::vector<std::string> &methods, const AP &... ap)
    {
        blue_print_.ROUTE(route, handler, methods, ap...);
    }

    template<typename... AP>
    void ROUTE(const char *route, int compute_queue_id, 
                const Handler &handler, 
                const std::vector<std::string> &methods, const AP &... ap)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, methods, ap...);
    }

    template<typename... AP>
    void GET(const char *route, const Handler &handler, const AP &... ap)
    {
        blue_print_.GET(route, handler, ap...);
    }

    template<typename... AP>
    void GET(const char *route, int compute_queue_id,
             const Handler &handler, const AP &... ap)
    {
        blue_print_.GET(route, compute_queue_id, handler, ap...);   
    }

    template<typename... AP>
    void POST(const char *route, const Handler &handler, const AP &... ap)
    {
        blue_print_.POST(route, handler, ap...);
    }

    template<typename... AP>
    void POST(const char *route, int compute_queue_id,
              const Handler &handler, const AP &... ap)
    {
        blue_print_.POST(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void DELETE(const char *route, const Handler &handler, const AP &... ap)
    {
        blue_print_.DELETE(route, handler, ap...);
    }

    template<typename... AP>
    void DELETE(const char *route, int compute_queue_id,
             const Handler &handler, const AP &... ap)
    {
        blue_print_.DELETE(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void PATCH(const char *route, const Handler &handler, const AP &... ap)
    {
        blue_print_.PATCH(route, handler, ap...);
    }

    template<typename... AP>
    void PATCH(const char *route, int compute_queue_id,
             const Handler &handler, const AP &... ap)
    {
        blue_print_.PATCH(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void PUT(const char *route, const Handler &handler, const AP &... ap)
    {
        blue_print_.PUT(route, handler, ap...);   
    }

    template<typename... AP>
    void PUT(const char *route, int compute_queue_id,
             const Handler &handler, const AP &... ap)
    {
        blue_print_.PUT(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void HEAD(const char *route, const Handler &handler, const AP &... ap)
    {
        blue_print_.HEAD(route, handler, ap...);
    }

    template<typename... AP>
    void HEAD(const char *route, int compute_queue_id,
             const Handler &handler, const AP &... ap)
    {
        blue_print_.HEAD(route, compute_queue_id, handler, ap...);
    }

public:
    void ROUTE(const char *route, const SeriesHandler &handler, Verb verb)
    {
        blue_print_.ROUTE(route, handler, verb);
    }

    void ROUTE(const char *route, int compute_queue_id, const SeriesHandler &handler, Verb verb)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, verb);
    }

    void ROUTE(const char *route, const SeriesHandler &handler, const std::vector<std::string> &methods)
    {
        blue_print_.ROUTE(route, handler, methods);
    }

    void ROUTE(const char *route, int compute_queue_id, 
                const SeriesHandler &handler, const std::vector<std::string> &methods)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, methods);
    }
    
    void GET(const char *route, const SeriesHandler &handler)
    {
        blue_print_.GET(route, handler);
    }

    void GET(const char *route, int compute_queue_id, const SeriesHandler &handler)
    {
        blue_print_.GET(route, compute_queue_id, handler);
    }

    void POST(const char *route, const SeriesHandler &handler)
    {
        blue_print_.POST(route, handler);
    }

    void POST(const char *route, int compute_queue_id, const SeriesHandler &handler)
    {
        blue_print_.POST(route, compute_queue_id, handler);
    }

    void DELETE(const char *route, const SeriesHandler &handler)
    {
        blue_print_.DELETE(route, handler);
    }

    void DELETE(const char *route, int compute_queue_id, const SeriesHandler &handler)
    {
        blue_print_.DELETE(route, compute_queue_id, handler);
    } 

    void PATCH(const char *route, const SeriesHandler &handler)
    {
        blue_print_.PATCH(route, handler);
    }

    void PATCH(const char *route, int compute_queue_id, const SeriesHandler &handler)
    {
        blue_print_.PATCH(route, compute_queue_id, handler);
    }

    void PUT(const char *route, const SeriesHandler &handler)
    {
        blue_print_.PUT(route, handler);
    }

    void PUT(const char *route, int compute_queue_id, const SeriesHandler &handler)
    {
        blue_print_.PUT(route, compute_queue_id, handler);
    }

    void HEAD(const char *route, const SeriesHandler &handler)
    {
        blue_print_.HEAD(route, handler);
    }

    void HEAD(const char *route, int compute_queue_id, const SeriesHandler &handler)
    {
        blue_print_.HEAD(route, compute_queue_id, handler);
    }

public:
    template<typename... AP>
    void ROUTE(const char *route, const SeriesHandler &handler, 
                Verb verb, const AP &... ap)
    {
        blue_print_.ROUTE(route, handler, verb, ap...);
    }

    template<typename... AP>
    void ROUTE(const char *route, int compute_queue_id, 
                const SeriesHandler &handler, Verb verb, const AP &... ap)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, verb, ap...);
    }

    template<typename... AP>
    void ROUTE(const char *route, const SeriesHandler &handler, 
                const std::vector<std::string> &methods, const AP &... ap)
    {
        blue_print_.ROUTE(route, handler, methods, ap...);
    }

    template<typename... AP>
    void ROUTE(const char *route, int compute_queue_id, 
                const SeriesHandler &handler, 
                const std::vector<std::string> &methods, const AP &... ap)
    {
        blue_print_.ROUTE(route, compute_queue_id, handler, methods, ap...);
    }
                
    template<typename... AP>
    void GET(const char *route, const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.GET(route, handler, ap...);
    }

    template<typename... AP>
    void GET(const char *route, int compute_queue_id,
             const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.GET(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void POST(const char *route, const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.POST(route, handler, ap...);
    }

    template<typename... AP>
    void POST(const char *route, int compute_queue_id,
              const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.POST(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void DELETE(const char *route, const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.DELETE(route, handler, ap...);
    }

    template<typename... AP>
    void DELETE(const char *route, int compute_queue_id,
             const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.DELETE(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void PATCH(const char *route, const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.PATCH(route, handler, ap...);
    }

    template<typename... AP>
    void PATCH(const char *route, int compute_queue_id,
             const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.PATCH(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void PUT(const char *route, const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.PUT(route, handler, ap...);
    }

    template<typename... AP>
    void PUT(const char *route, int compute_queue_id,
             const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.PUT(route, compute_queue_id, handler, ap...);
    }

    template<typename... AP>
    void HEAD(const char *route, const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.HEAD(route, handler, ap...);
    }

    template<typename... AP>
    void HEAD(const char *route, int compute_queue_id,
             const SeriesHandler &handler, const AP &... ap)
    {
        blue_print_.HEAD(route, compute_queue_id, handler, ap...);
    }

public:
    void Static(const char *relative_path, const char *root);

    void list_routes();

    void register_blueprint(const BluePrint &bp, const std::string &url_prefix);
    
    template <typename... AP>
    void Use(AP &&...ap)
    {
        auto *tp = new std::tuple<AP...>(std::move(ap)...);
        for_each(*tp, GlobalAspectFunc());
    }
    void SetServerName(const std::string& name)
    {
        serverName = name;
    }
public:
    HttpServer() :
            WFServer(std::bind(&HttpServer::process, this, std::placeholders::_1))
    {
        serverName = "Sxb Server";
    }

    HttpServer &max_connections(size_t max_connections)
    {
        this->params.max_connections = max_connections;
        return *this;
    }

		size_t get_max_connections()const
		{
			return this->params.max_connections;
		}

		HttpServer& peer_response_timeout(int peer_response_timeout)
		{
			this->params.peer_response_timeout = peer_response_timeout;
			return *this;
		}

		int get_peer_response_timeout()const
		{
			return this->params.peer_response_timeout;
		}

		HttpServer& receive_timeout(int receive_timeout)
		{
			this->params.receive_timeout = receive_timeout;
			return *this;
		}

		int get_receive_timeout()const
		{
			return this->params.receive_timeout;
		}

		HttpServer& keep_alive_timeout(int keep_alive_timeout)
		{
			this->params.keep_alive_timeout = keep_alive_timeout;
			return *this;
		}

		int get_keep_alive_timeout()const
		{
			return this->params.keep_alive_timeout;
		}

		HttpServer& request_size_limit(size_t request_size_limit)
		{
			this->params.request_size_limit = request_size_limit;
			return *this;
		}

		size_t get_request_size_limit()const
		{
			return this->params.request_size_limit;
		}

		HttpServer& ssl_accept_timeout(int ssl_accept_timeout)
		{
			this->params.ssl_accept_timeout = ssl_accept_timeout;
			return *this;
		}

		int get_ssl_accept_timeout()const
		{
			return this->params.ssl_accept_timeout;
		}

		using TrackFunc = std::function<void(HttpTask* server_task)>;

		HttpServer& track();

		HttpServer& track(const TrackFunc& track_func);

		HttpServer& track(TrackFunc&& track_func);

	protected:
		CommSession* new_session(long long seq, CommConnection* conn) override;

	private:
		void process(HttpTask* task);

		int serve_static(const char* path, OUT BluePrint& bp);

		struct GlobalAspectFunc
		{
			template <typename T>
			void operator()(T& t) const
			{
				Aspect* asp = new T(std::move(t));
				GlobalAspect* global_aspect = GlobalAspect::get_instance();
				global_aspect->aspect_list.push_back(asp);
			}
		};

	private:
		BluePrint blue_print_;
		TrackFunc track_func_;
		std::string serverName;
	};

}  // namespace wfrest

#endif // WFREST_HTTPSERVER_H_