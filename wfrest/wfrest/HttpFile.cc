#include "workflow/WFTaskFactory.h"

#include <sys/stat.h>

#include "wfrest/HttpFile.h"
#include "wfrest/HttpMsg.h"
#include "wfrest/PathUtil.h"
#include "wfrest/HttpServerTask.h"
#include "wfrest/FileUtil.h"
#include "wfrest/ErrorCode.h"

using namespace wfrest;

namespace
{
	struct pread_multi_context
	{
		std::string file_name;
		int file_index;
		bool last = false;
		HttpResp* resp;
		std::string multipart_start;
		std::string multipart_end;
	};
	/*
	We do not occupy any thread to read the file, but generate an asynchronous file reading task
	and reply to the request after the reading is completed.

	We need to read the whole data into the memory before we start replying to the message.
	Therefore, it is not suitable for transferring files that are too large.

	todo : Any better way to transfer large File?
	*/
	void pread_callback(WFFileIOTask* pread_task)
	{
		FileIOArgs* args = pread_task->get_args();
		long ret = pread_task->get_retval();
		//auto* resp = static_cast<HttpResp*>(pread_task->user_data);
		auto* ctx = static_cast<pread_multi_context*>(pread_task->user_data);
		HttpResp* resp = ctx->resp;
		// clear output body
		if (pread_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
		{
			char res[256];
			resp->set_status_code("503");
			//resp->append_output_body_nocopy("503 Internal Server Error\n", 26);
			//resp->Error(StatusFileReadError);
			sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 503, "503 Internal Server Error", ctx->file_name.substr(ctx->file_name.find_last_of("/")).c_str());
			resp->append_output_body(res, strlen(res));
		}
		else
		{
			resp->append_output_body(args->buf, ret);
		}
	}



	std::string build_multipart_start(const std::string& file_name, int idx)
	{
		std::string str;
		str.reserve(256);  // reserve space avoid copy
		// todo : how to generate name ?
		str.append("--");
		str.append(MultiPartForm::k_default_boundary);
		str.append("\r\n");
		str.append("Content-Disposition: form-data; name=");
		str.append("\"file");
		str.append(std::to_string(idx));
		str.append("\"; filename=\"");
		str.append(file_name);
		str.append("\"");

		const char* suffix = strrchr(file_name.c_str(), '.');
		if (suffix)
		{
			std::string stype = ContentType::to_str_by_suffix(++suffix);
			if (!stype.empty())
			{
				str.append("\r\n");
				str.append("Content-Type: ");
				str.append(stype);
			}
			else
			{
				str.append("\r\n");
				str.append("Content-Type: application/octet-stream");
			}
		}
		str.append("\r\n\r\n");
		return str;
	}

	std::string build_multipart_end()
	{
		std::string multi_part_last;
		multi_part_last.reserve(128);
		multi_part_last.append("--");
		// RFC1521 says that a boundary "must be no longer than 70 characters, not counting the two leading hyphens".
		multi_part_last.append(MultiPartForm::k_default_boundary);
		multi_part_last.append("--");
		return multi_part_last;
	}

	void pread_multi_callback(WFFileIOTask* pread_task)
	{
		FileIOArgs* args = pread_task->get_args();
		long ret = pread_task->get_retval();
		auto* ctx = static_cast<pread_multi_context*>(pread_task->user_data);
		HttpResp* resp = ctx->resp;

		std::string multipart_start = build_multipart_start(ctx->file_name, ctx->file_index);
		ctx->multipart_start = std::move(multipart_start);
		resp->append_output_body(ctx->multipart_start.c_str(), ctx->multipart_start.size());
		if (pread_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
		{
			char res[256];
			resp->set_status_code("503");
			//resp->append_output_body_nocopy("503 Internal Server Error\n", 26);
			//resp->Error(StatusFileReadError);
			sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 503, "503 Internal Server Error", ctx->file_name.substr(ctx->file_name.find_last_of("/")).c_str());
			resp->append_output_body(res, strlen(res));
		}
		else
		{
			resp->append_output_body(args->buf, ret);
		}

		if (ctx->last)   // last one
		{
			std::string multipart_end = build_multipart_end();
			ctx->multipart_end = std::move(multipart_end);
			resp->append_output_body(ctx->multipart_end.c_str(), ctx->multipart_end.size());
		}
	}

	void pwrite_callback(WFFileIOTask* pwrite_task)
	{
		long ret = pwrite_task->get_retval();
		HttpServerTask* server_task = task_of(pwrite_task);
		HttpResp* resp = server_task->get_resp();
		auto* ctx = static_cast<pread_multi_context*>(pwrite_task->user_data);
		//delete static_cast<std::string*>(pwrite_task->user_data);	
		
		char res[256];
		if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
		{
			resp->set_status_code("503");
			//resp->append_output_body_nocopy("503 Internal Server Error\n", 26);
			//resp->Error(StatusFileWriteError);
			sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 503, "503 Internal Server Error", ctx->file_name.substr(ctx->file_name.find_last_of("/")).c_str());
			resp->append_output_body(res, strlen(res));
		}
		else
		{
			//resp->append_output_body_nocopy("Save 200 success\n", 17);
			sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 503, "503 Internal Server Error", ctx->file_name.substr(ctx->file_name.find_last_of("/")).c_str());
			resp->append_output_body(res, strlen(res));
		}
		delete static_cast<pread_multi_context*>(pwrite_task->user_data);
	}

}  // namespace

// note : [start, end)
int HttpFile::send_file(const std::string &path, size_t file_start, size_t file_end, HttpResp *resp)
{
	if(!PathUtil::is_file(path))
    {
        return StatusNotFound;
    }
    int start = file_start;
    int end = file_end;
	if (end == -1 || start < 0)
	{
		size_t file_size;
		int ret = FileUtil::size(path, OUT & file_size);

		if (ret != StatusOK)
		{
			char res[256];
			resp->set_status(404);
			//resp->headers["Content-Type"] = "text/html";
			//resp->append_output_body_nocopy("404 File NOT FOUND\n", 19);
			sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 404, "404 File NOT FOUND", path.substr(path.find_last_of("/") + 1).c_str());
			resp->append_output_body(res, strlen(res));
			return ret;
		}
		if (end == -1) end = file_size;
		if (start < 0) start = file_size + start;
	}

    if (end <= start)
    {
        return StatusFileRangeInvalid;
    }

    http_content_type content_type = CONTENT_TYPE_NONE;
    std::string suffix = PathUtil::suffix(path);
    if(!suffix.empty())
    {
        content_type = ContentType::to_enum_by_suffix(suffix);
    }
    if (content_type == CONTENT_TYPE_NONE || content_type == CONTENT_TYPE_UNDEFINED) {
        content_type = APPLICATION_OCTET_STREAM;
    }
    resp->headers["Content-Type"] = ContentType::to_str(content_type);

    size_t size = end - start;
    void *buf = malloc(size);

    HttpServerTask *server_task = task_of(resp);
    server_task->add_callback([buf](HttpTask *server_task)
                              {
                                  free(buf);
                              });
    // https://datatracker.ietf.org/doc/html/rfc7233#section-4.2
    // Content-Range: bytes 42-1233/1234
    resp->headers["Content-Range"] = "bytes " + std::to_string(start)
                                            + "-" + std::to_string(end)
                                            + "/" + std::to_string(size);

#ifndef OS_WINDOWS
 WFFileIOTask *pread_task = WFTaskFactory::create_pread_task(path,
                                                                buf,
                                                                size,
                                                                static_cast<off_t>(start),
                                                                pread_callback);
    pread_task->user_data = resp;  
    **server_task << pread_task;
#else
	FILE* f = fopen(path.c_str(), "rb");
	if (f == NULL)
	{
		char res[256];
		resp->set_status(404);
		//resp->append_output_body("404 File NOT FOUND\n", 19);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 404, "404 File NOT FOUND", path.substr(path.find_last_of("/")).c_str());
		resp->append_output_body(res, strlen(res));
		return StatusNotFound;
	}
	else
	{
		int bufLen = fread(buf, 1, size, f);
		fclose(f);
		resp->append_output_body(buf, bufLen);
	}
#endif
	return StatusOK;
}

int HttpFile::send_file_for_multi(const std::vector<std::string>& path_list, int path_idx, HttpResp* resp)
{
	HttpServerTask* server_task = task_of(resp);
	const std::string& file_path = path_list[path_idx];
	//LOG_DEBUG << "File Path : " << file_path;

	auto* ctx = new pread_multi_context;
	ctx->file_name = PathUtil::base(file_path);
	ctx->file_index = path_idx;
	ctx->resp = resp;

	if (path_idx == path_list.size() - 1)
	{
		// last one
		ctx->last = true;
	}

	size_t size;
	int ret = FileUtil::size(file_path, OUT & size);
	if (ret != StatusOK)
	{
		char res[256];
		resp->set_status(404);
		//resp->append_output_body("404 File NOT FOUND\n", 19);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 404, "404 File NOT FOUND", file_path.substr(file_path.find_last_of("/")).c_str());
		resp->append_output_body(res, strlen(res));
		return StatusNotFound;
	}
	void* buf = malloc(size);
	server_task->add_callback([buf, ctx](HttpTask* server_task)
		{
			free(buf);
			delete ctx;
		});
#ifndef OS_WINDOWS
	WFFileIOTask* pread_task = WFTaskFactory::create_pread_task(file_path,
		buf,
		size,
		0,
		pread_multi_callback);
	pread_task->user_data = ctx;
	**server_task << pread_task;
#else
	FILE* f = fopen(file_path.c_str(), "rb");
	if (f == NULL)
	{
		char res[256];
		resp->set_status(404);
		//resp->append_output_body_nocopy("503 Internal Server Error\n", 26);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 404, "404 File NOT FOUND", file_path.substr(file_path.find_last_of("/")).c_str());
		resp->append_output_body(res, strlen(res));
		return StatusNotFound;
	}
	else
	{
		int bufLen = fread(buf, 1, size, f);
		fclose(f);
		std::string multipart_start = build_multipart_start(ctx->file_name, ctx->file_index);
		ctx->multipart_start = std::move(multipart_start);
		resp->append_output_body(ctx->multipart_start.c_str(), ctx->multipart_start.size()); //boundary
		resp->append_output_body(buf, bufLen); //文件数据
		if (ctx->last)   // last one
		{
			std::string multipart_end = build_multipart_end();
			ctx->multipart_end = std::move(multipart_end);
			resp->append_output_body(ctx->multipart_end.c_str(), ctx->multipart_end.size());
		}
	}
#endif
	return StatusOK;
}

int HttpFile::send_file_for_multi(const std::vector<std::string>& path_list, int path_idx, protocol::HttpRequest* req)
{
	//HttpServerTask* server_task = task_of(req);
	const std::string& file_path = path_list[path_idx];
	//LOG_DEBUG << "File Path : " << file_path;

	auto* ctx = new pread_multi_context;
	ctx->file_name = PathUtil::base(file_path);
	ctx->file_index = path_idx;

	if (path_idx == path_list.size() - 1)
	{
		// last one
		ctx->last = true;
	}

	size_t size;
	int ret = FileUtil::size(file_path, OUT & size);
	if (ret != StatusOK)
	{
		//resp->set_status(404);
		//resp->append_output_body_nocopy("404 File NOT FOUND\n", 19);
		//LOG_ERROR << "File Path : " << file_path << ",get file size error";
		return ret;
	}
	void* buf = malloc(size);
/*#ifdef OS_WINDOWS
	WFFileIOTask* pread_task = WFTaskFactory::create_pread_task(file_path,
		buf,
		size,
		0,
		pread_multi_callback);
	pread_task->user_data = ctx;
	**server_task << pread_task;
#else*/
	FILE* f = fopen(file_path.c_str(), "rb");
	if (f == NULL)
	{
		//resp->set_status_code("404");
		//resp->append_output_body_nocopy("404 File NOT FOUND\n", 19);
		//LOG_ERROR << "File Path : " << file_path << ",open file error";
		free(buf);
		delete ctx;
		return StatusNotFound;
	}
	else
	{
		std::string body;
		int bufLen = fread(buf, 1, size, f);
		fclose(f);
		req->add_header_pair("Content-Type", "multipart/form-data boundary="+ MultiPartForm::k_default_boundary);
		std::string multipart_start = build_multipart_start(ctx->file_name, ctx->file_index);
		std::string multipart_end;
		if (ctx->last)   // last one
		{
			multipart_end = build_multipart_end();
		}	
		body = multipart_start  + std::string((char*)buf, bufLen) + multipart_end;
		req->append_output_body(body.data(), body.size());
	}
	free(buf);
	delete ctx;
//#endif
}

void HttpFile::save_file(const std::string& dst_path, const std::string& content, HttpResp* resp)
{
	HttpServerTask* server_task = task_of(resp);

	//auto* save_content = new std::string;
	//*save_content = content;
	auto* ctx = new pread_multi_context;
	ctx->file_name = PathUtil::base(dst_path);
	ctx->multipart_start = content; //借用变量
	ctx->resp = resp;
#ifndef OS_WINDOWS
	WFFileIOTask* pwrite_task = WFTaskFactory::create_pwrite_task(dst_path,
		static_cast<const void*>(save_content->c_str()),
		save_content->size(),
		0,
		pwrite_callback);
	**server_task << pwrite_task;
	pwrite_task->user_data = ctx;//save_content;
#else
	char res[256];
	FILE* f = fopen(dst_path.c_str(), "wb");
	if (f == NULL)
	{
		resp->set_status(404);
		//resp->append_output_body_nocopy("404 File NOT FOUND\n", 19);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 404, "404 File NOT FOUND", dst_path.c_str());
		resp->append_output_body(res, strlen(res));
	}
	else
	{
		int bufLen = fwrite(content.c_str(), 1, content.size(), f);
		fclose(f);
		//resp->append_output_body_nocopy("Save 200 success\n", 17);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 0, "success", dst_path.c_str());
		resp->append_output_body(res, strlen(res));
	}
#endif
}

void HttpFile::save_file(const std::string& dst_path, std::string&& content, HttpResp* resp)
{
	HttpServerTask* server_task = task_of(resp);

	//auto* save_content = new std::string;
	//*save_content = std::move(content);
	auto* ctx = new pread_multi_context;
	ctx->file_name = PathUtil::base(dst_path);
	ctx->multipart_start = content; //借用变量
	ctx->resp = resp;
#ifndef OS_WINDOWS
	WFFileIOTask* pwrite_task = WFTaskFactory::create_pwrite_task(dst_path,
		static_cast<const void*>(save_content->c_str()),
		save_content->size(),
		0,
		pwrite_callback);
	**server_task << pwrite_task;
	pwrite_task->user_data = ctx;//save_content;
#else
	char res[256];
	FILE* f = fopen(dst_path.c_str(), "wb");
	if (f == NULL)
	{
		resp->set_status(404);
		//resp->append_output_body_nocopy("404 File NOT FOUND\n", 19);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 404, "404 File NOT FOUND", dst_path.c_str());
		resp->append_output_body(res, strlen(res));
	}
	else
	{
		int bufLen = fwrite(content.c_str(), 1, content.size(), f);
		fclose(f);
		//resp->append_output_body_nocopy("Save 200 success\n", 17);
		sprintf(res, "{\"code\":%d,\"msg\":\"%s\",\"fileName\":\"%s\"}", 0, "success", dst_path.c_str());
		resp->append_output_body(res, strlen(res));
	}
#endif
}