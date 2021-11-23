#ifndef __webservice_cc__
#define __webservice_cc__

#include "webservice.h"
#include "http_parser.h"

ACE_Message_Block* MicroService::handle_OPTIONS(ACE_Message_Block& in)
{
    std::string http_header;
    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    http_header += "Access-Control-Allow-Headers: DNT, User-Agent, X-Requested-With, If-Modified-Since, Cache-Control, Content-Type, Range\r\n";
    http_header += "Access-Control-Max-Age: 1728000\r\n";
    http_header += "Access-Control-Allow-Origin: *\r\n";
    http_header += "Content-Type: text/plain; charset=utf-8\r\n";
    http_header += "Content-Length: 0\r\n";
    http_header += "\r\n\r\n";
    ACE_Message_Block* rsp = nullptr;

    ACE_NEW_RETURN(rsp, ACE_Message_Block(256), nullptr);

    std::memcpy(rsp->wr_ptr(), http_header.c_str(), http_header.length());
    rsp->wr_ptr(http_header.length());

    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response %s \n"), http_header.length(), http_header.c_str()));
    return(rsp);
}

ACE_Message_Block* MicroService::handle_GET(ACE_Message_Block& in, Mongodbc& dbInst)
{
    std::string http_header, http_body;
    ACE_Message_Block* rsp = nullptr;

    http_body = "{\"name\": \"Testing Now\"}";

    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Content-Length: " + std::to_string(http_body.length()) + "\r\n";
    http_header += "Content-Type: text/html\r\n";
    http_header += "Connection: keep-alive\r\n";

    ACE_NEW_RETURN(rsp, ACE_Message_Block(256), nullptr);

    std::memcpy(rsp->wr_ptr(), http_header.c_str(), http_header.length());
    rsp->wr_ptr(http_header.length());
    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response %s \n"), http_header.length(), http_header.c_str()));

    return(rsp);
}

ACE_Message_Block* MicroService::handle_POST(ACE_Message_Block& in, Mongodbc& dbInst)
{
    std::string http_header;
    ACE_Message_Block* rsp = nullptr;

    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Connection: keep-alive\r\n";
    http_header += "Content-Length: 0\r\n";

    ACE_NEW_RETURN(rsp, ACE_Message_Block(256), nullptr);

    std::memcpy(rsp->wr_ptr(), http_header.c_str(), http_header.length());
    rsp->wr_ptr(http_header.length());

    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response %s \n"), http_header.length(), http_header.c_str()));
    return(rsp);
}

ACE_Message_Block* MicroService::handle_PUT(ACE_Message_Block& in, Mongodbc& dbInst)
{
    std::string http_header, http_body;
    ACE_Message_Block* rsp = nullptr;

    http_body = "<html><title>HHHHHH</title><body><h5>Hello Naushad</h5></body>";

    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Access-Control-Allow-Origin: *\r\n";
    http_header += "Access-Control-Allow-Methods: GET,POST,OPTIONS,DELETE,PUT\r\n";
    http_header += "Content-Length: " + std::to_string(http_body.length()) + "\r\n";
    http_header += "Content-Type: text/html\r\n";
    http_header += "Connection: keep-alive\r\n";

}

ACE_Message_Block* MicroService::handle_DELETE(ACE_Message_Block& in, Mongodbc& dbInst)
{
    std::string http_header, http_body;
    ACE_Message_Block* rsp = nullptr;

    http_body = "<html><title>HHHHHH</title><body><h5>Hello Naushad</h5></body>";

    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Access-Control-Allow-Origin: *\r\n";
    http_header += "Access-Control-Allow-Methods: GET,POST,OPTIONS,DELETE,PUT\r\n";
    http_header += "Content-Length: " + std::to_string(http_body.length()) + "\r\n";
    http_header += "Content-Type: text/html\r\n";
    http_header += "Connection: keep-alive\r\n";

}

ACE_INT32 MicroService::process_request(ACE_HANDLE handle, ACE_Message_Block& mb, Mongodbc& dbInst)
{
    std::string http_header, http_body;
    http_header.clear();
    http_body.clear();
    ACE_Message_Block* rsp = nullptr;

    std::string req(mb.rd_ptr(), mb.length());

    if(std::string::npos != req.find("OPTIONS", 0)) {
      //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l HTTP Method is OPTIONS\n")));
      rsp = handle_OPTIONS(req);
      //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response\n%s"), rsp->length(), rsp->rd_ptr()));
    } else if(std::string::npos != req.find("GET", 0)) {
      rsp = handle_GET(req, dbInst); 
    } else if(std::string::npos != req.find("POST", 0)) {
      rsp = handle_POST(req, dbInst);
    } else if(std::string::npos != req.find("PUT", 0)) {
      rsp = handle_PUT(req, dbInst);
    } else if(std::string::npos != req.find("DELETE", 0)) {
      rsp = handle_DELETE(mb, dbInst);  
    } else {
      /* Not supported Method */
    }

    std::int32_t ret = 0;
    if(nullptr != rsp) {

      std::string response(rsp->rd_ptr(), rsp->length());
      rsp->release();
      std::int32_t  toBeSent = response.length();
      std::int32_t offset = 0;
      do {
        ret = send(handle, (response.c_str() + offset), (toBeSent - offset), 0);
        if(ret < 0) {
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l sent to peer is failed\n")));
          break;
        }
        offset += ret;
        ret = 0;
      } while((toBeSent != offset));

      //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d bytes sent %d\n"), response.length(), offset));
    }
    return(ret);
}

std::string MicroService::get_contentType(std::string ext)
{
    std::string cntType("");
    /* get the extension now for content-type */
    if(!ext.compare("woff")) {
      cntType = "font/woff";
    } else if(!ext.compare("woff2")) {
      cntType = "font/woff2";
    } else if(!ext.compare("ttf")) {
      cntType = "font/ttf";
    } else if(!ext.compare("otf")) {
      cntType = "font/otf";
    } else if(!ext.compare("css")) {
      cntType = "text/css";
    } else if(!ext.compare("js")) {
      cntType = "text/javascript";
    } else if(!ext.compare("eot")) {
      cntType = "application/vnd.ms-fontobject";
    } else if(!ext.compare("html")) {
      cntType = "text/html";
    } else if(!ext.compare("svg")) {
      cntType = "image/svg+xml";
    } else if(!ext.compare("gif")) {
      cntType ="image/gif";
    } else if(!ext.compare("png")) {
      cntType = "image/png";
    } else if(!ext.compare("ico")) {
      cntType = "image/vnd.microsoft.icon";
    } else if(!ext.compare("jpg")) {
      cntType = "image/jpeg";
    } else if(!ext.compare("json")) {
      cntType = "application/json";
    } else {
      cntType = "text/html";
    }
    return(cntType);
}

ACE_Message_Block* MicroService::handle_POST(std::string& in, Mongodbc& dbInst)
{
    /* Check for Query string */
    Http http(in);
    /* Action based on uri in get request */
    std::string uri(http.get_uriName());

    if(!uri.compare("/api/shipping")) {
        std::string collectionName("shipping");
        std::string content = http.body();
        if(content.length()) {
            bool record = dbInst.create_shipment(content);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l New Document for shipping ->\n %s\n"), content.c_str()));
        } 
    } else if(!uri.compare("/api/account")) {
        std::string collectionName("account");
        std::string content = http.body();
        if(content.length()) {
            std::string record = dbInst.create_account(content);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l New Document for account ->\n %s\n"), content.c_str()));
        }
    }
    return(build_responseOK(std::string()));
}

ACE_Message_Block* MicroService::handle_GET(std::string& in, Mongodbc& dbInst)
{
    size_t ct_offset = 0, cl_offset = 0;
    /* Check for Query string */
    Http http(in);

    /* Action based on uri in get request */
    std::string uri(http.get_uriName());

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l API Name %s\n"), uri.c_str()));
    if(!uri.compare("/api/login")) {
        std::string collectionName("account");

        /* user is trying to log in - authenticate now */
        auto user = http.get_element("userId");
	    auto pwd = http.get_element("password");

        if(user.length() && pwd.length()) {
            /* do an authentication with DB now */
            std::string document = "{\"accountCode\" : \"" +  user + "\", " + 
                                   "\"accountPassword\" : \"" + pwd + "\"" + 
                                    "}";
            //std::string projection("{\"accountCode\" : true, \"_id\" : false}");
            std::string projection("{\"_id\" : false}");
            std::string record = dbInst.validate_user(collectionName, document, projection);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l User or Customer details %s\n"), record.c_str()));
            return(build_responseOK(record));
        } 
    } else if(!uri.compare("/api/account")) {
        std::string collectionName("account");

        /* user is trying to log in - authenticate now */
        auto user = http.get_element("accountCode");

        if(user.length()) {
            /* do an authentication with DB now */
            std::string document = "{\"accountCode\" : \"" +  user + "\" " + 
                                    "}";
            //std::string projection("{\"accountCode\" : true, \"_id\" : false}");
            std::string projection("{\"_id\" : false}");
            std::string record = dbInst.get_accountInfo(collectionName, document, projection);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Customer Account Info %s\n"), record.c_str()));
            return(build_responseOK(record));
        }

    } else if(!uri.compare("/api/shipping") || (!uri.compare("/api/shipment"))) {
        std::string collectionName("shipping");
        auto awbNo = http.get_element("shipmentNo");
        auto accountCode = http.get_element("accountCode");

        if(awbNo.length() && accountCode.length()) {
            /* do an authentication with DB now */
            std::string document = "{\"accountCode\" : \"" +
                                     accountCode + "\" ," +
                                    "\"shipmentNo\" : \"" +
                                    awbNo + "\"" +
                                    "}";
           std::string projection("{\"_id\" : false}");
            std::string record = dbInst.get_shipment(collectionName, document, projection);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l AWB Way Bills %s\n"), record.c_str()));
            return(build_responseOK(record));
        }
    } else if((!uri.compare("/api/altref"))) {
        std::string collectionName("shipping");
        auto altRefNo = http.get_element("altRefNo");

        if(altRefNo.length()) {
            /* do an authentication with DB now */
            std::string document = "{\"altRefNo\" : \"" +
                                     altRefNo + "\" " +
                                    "}";
           std::string projection("{\"_id\" : false}");
            std::string record = dbInst.get_shipment(collectionName, document, projection);
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l altRefNo Response %s\n"), record.c_str()));
            return(build_responseOK(record));
        }
    } else if((!uri.compare(0, 6, "/bayt/"))) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l frontend Request %s\n"), uri.c_str()));
        /* build the file name now */
        std::string fileName("");
        std::string ext("");

        std::size_t found = uri.find_last_of(".");
        if(found != std::string::npos) {
          ext = uri.substr((found + 1), (uri.length() - found));
          fileName = uri.substr(6, (uri.length() - 6));
          std::string newFile = "../webgui/ui/" + fileName;
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l newFile Name is %s The extension is %s\n"), newFile.c_str(), ext.c_str()));
          /* Open the index.html file and send it to web browser. */
          std::ifstream ifs(newFile.c_str());
          std::stringstream _str("");

          if(ifs.is_open()) {
              ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Request file %s - open successfully.\n"), uri.c_str()));
              std::string cntType("");
              cntType = get_contentType(ext); 

              _str << ifs.rdbuf();
              ifs.close();
              return(build_responseOK(_str.str(), cntType));
          }
        }
    } else if(!uri.compare(0, 8, "/assets/")) {
        /* build the file name now */
        std::string fileName("");
        std::string ext("");

        std::size_t found = uri.find_last_of(".");
        if(found != std::string::npos) {
          ext = uri.substr((found + 1), (uri.length() - found));
          std::string newFile = "../webgui/ui" + uri;
          ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l newFile Name is %s The extension is %s\n"), newFile.c_str(), ext.c_str()));
          /* Open the index.html file and send it to web browser. */
          std::ifstream ifs(newFile.c_str(), ios::binary);
          std::stringstream _str("");
          std::string cntType("");

          if(ifs.is_open()) {
              ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Request file %s - open successfully.\n"), uri.c_str()));

              cntType = get_contentType(ext);
              _str << ifs.rdbuf();
              ifs.close();

              return(build_responseOK(_str.str(), cntType));
          }
        }

    } else if((!uri.compare(0, 6, "/bayt/"))) {
        std::string newFile = "../webgui/ui/index.html";
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l newFile Name is %s \n"), newFile.c_str()));
        /* Open the index.html file and send it to web browser. */
        std::ifstream ifs(newFile.c_str(), ios::binary);
        std::stringstream _str("");
        std::string cntType("");

        if(ifs.is_open()) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Request file %s - open successfully.\n"), uri.c_str()));

            cntType = "text/html";
            _str << ifs.rdbuf();
            ifs.close();

            return(build_responseOK(_str.str(), cntType));
        }

    } else if(!uri.compare(0, 1, "/")) {
        std::string newFile = "../webgui/ui/index.html";
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l newFile Name is %s \n"), newFile.c_str()));
        /* Open the index.html file and send it to web browser. */
        std::ifstream ifs(newFile.c_str(), ios::binary);
        std::stringstream _str("");
        std::string cntType("");

        if(ifs.is_open()) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Request file %s - open successfully.\n"), uri.c_str()));

            cntType = "text/html";
            _str << ifs.rdbuf();
            ifs.close();

            return(build_responseOK(_str.str(), cntType));
        }
    }

    return(build_responseOK(std::string()));
}

ACE_Message_Block* MicroService::handle_PUT(std::string& in, Mongodbc& dbInst)
{
    /* Check for Query string */
    Http http(in);

    /* Action based on uri in get request */
    std::string uri(http.get_uriName());

    if(!uri.compare("/api/shipment")) {
        /** Update on Shipping */
        std::string content = http.body();
        std::string awbNo = http.get_element("shipmentNo");
        std::string accountCode = http.get_element("accountCode");
        std::string query = "{\"accountCode\" : \"" +
                             accountCode + "\" ," +
                             "\"shipmentNo\" : \"" +
                             awbNo + "\"" +
                             "}";

        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Updating Shipment %s\n"), query.c_str()));
        dbInst.update_shipment(query, content);
    }
#if 0
    size_t ct_offset = 0, cl_offset = 0;
    /* Get the payload length */
    if(std::string::npos != (ct_offset = in.find("Content-Type: ", 0)) && std::string::npos != (cl_offset = in.find("Content-Length: ", 0))) {
        /* Both content Type & content Length are present */
        size_t ct_offset_v = 0, cl_offset_v = 0;
        ct_offset_v = in.find("\r\n", ct_offset);
        cl_offset_v = in.find("\r\n", cl_offset);
        std::string ct_match("Content-Type: ");
        ct_offset += ct_match.length();
        std::string cl_match("Content-Length: ");
        cl_offset += cl_match.length();

        std::string ct_value = in.substr(ct_offset, (ct_offset_v - ct_offset));
        std::string cl_value = in.substr(cl_offset, (cl_offset_v - cl_offset));

        if(!ct_value.compare("application/json")) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l The content Type is application/json\n")));
            std::string body_delimeter("\r\n\r\n");
            size_t body_offset = in.find(body_delimeter, 0);
            if(std::string::npos != body_offset) {
                body_offset += body_delimeter.length();
                std::string document = in.substr(body_offset);

                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Document is writen in databse\n")));
                /* write this document into data base now.*/
                dbInst.create_shipment(document);
            }
        }
    }
#endif
    return(build_responseOK(std::string()));
}

ACE_Message_Block* MicroService::handle_OPTIONS(std::string& in)
{
    std::string http_header;
    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
    http_header += "Access-Control-Allow-Headers: DNT, User-Agent, X-Requested-With, If-Modified-Since, Cache-Control, Content-Type, Range\r\n";
    http_header += "Access-Control-Max-Age: 1728000\r\n";
    http_header += "Access-Control-Allow-Origin: *\r\n";
    http_header += "Content-Type: text/plain; charset=utf-8\r\n";
    http_header += "Content-Length: 0\r\n";
    http_header += "\r\n\r\n";
    ACE_Message_Block* rsp = nullptr;

    ACE_NEW_RETURN(rsp, ACE_Message_Block(512), nullptr);

    std::memcpy(rsp->wr_ptr(), http_header.c_str(), http_header.length());
    rsp->wr_ptr(http_header.length());

    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response %s \n"), http_header.length(), http_header.c_str()));
    return(rsp);
}

ACE_Message_Block* MicroService::build_responseCreated()
{
    std::string http_header;
    ACE_Message_Block* rsp = nullptr;

    http_header = "HTTP/1.1 201 Created\r\n";
    http_header += "Connection: keep-alive\r\n";
    http_header += "Access-Control-Allow-Origin: *\r\n";
    http_header += "Content-Length: 0\r\n";

    ACE_NEW_RETURN(rsp, ACE_Message_Block(256), nullptr);

    std::memcpy(rsp->wr_ptr(), http_header.c_str(), http_header.length());
    rsp->wr_ptr(http_header.length());

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response %s \n"), http_header.length(), http_header.c_str()));
    return(rsp);
}

ACE_Message_Block* MicroService::build_responseOK(std::string httpBody, std::string contentType)
{
    std::string http_header;
    ACE_Message_Block* rsp = nullptr;

    http_header = "HTTP/1.1 200 OK\r\n";
    http_header += "Connection: keep-alive\r\n";
    http_header += "Access-Control-Allow-Origin: *\r\n";

    if(httpBody.length()) {
        http_header += "Content-Length: " + std::to_string(httpBody.length()) + "\r\n";
        http_header += "Content-Type: " + contentType + "\r\n";
        http_header += "\r\n";

    } else {
        http_header += "Content-Length: 0\r\n";
    }

    ACE_NEW_RETURN(rsp, ACE_Message_Block(std::size_t(MemorySize::SIZE_1KB) + httpBody.length()), nullptr);

    std::memcpy(rsp->wr_ptr(), http_header.c_str(), http_header.length());
    rsp->wr_ptr(http_header.length());
    std::memcpy(rsp->wr_ptr(), httpBody.c_str(), httpBody.length());
    rsp->wr_ptr(httpBody.length());

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l respone length %d response header\n%s"), (http_header.length() + httpBody.length()), http_header.c_str()));
    return(rsp);
}

ACE_INT32 MicroService::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Micro service gets signal %d\n"), signum));
    m_continue = false;

    return(0);
}

int MicroService::open(void *arg)
{
    /*! Number of threads are 5, which is 2nd argument. */
    activate();
    return(0);
}

int MicroService::close(u_long flag)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Micro service is closing\n")));
    return(0);
}

/*
 * @brief: This function is the entry point for Thread. Once the thread is spawned, control comes here
 *         and It blocks on message queue. The thread is termed as Worker.
 * @param: none
 * @return: 
 */
int MicroService::svc() 
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Micro service is spawned\n")));
    while(m_continue) {
        if(-1 != getq(m_mb)) {
            switch (m_mb->msg_type())
            {
            case ACE_Message_Block::MB_DATA:
            {
                /*_ _ _ _ _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
                 | 4-bytes handle   | 4-bytes db instance pointer   | request (payload) |
                 |_ _ _ _ _ _ _ _ _ |_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _|
                */
                ACE_HANDLE handle = *((ACE_HANDLE *)m_mb->rd_ptr());
                m_mb->rd_ptr(sizeof(ACE_HANDLE));

                std::uintptr_t inst = *((std::uintptr_t *)m_mb->rd_ptr());
                Mongodbc* dbInst = reinterpret_cast<Mongodbc*>(inst);
                m_mb->rd_ptr(sizeof(uintptr_t));

                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l URI %s dbName %s\n"), dbInst->get_uri().c_str(), dbInst->get_dbName().c_str()));
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l handle %d length %d \n"), handle, m_mb->length()));

                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l httpReq\n %s"), m_mb->rd_ptr()));
                /*! Process The Request */
                process_request(handle, *m_mb, *dbInst);
                m_mb->release();
                break;
            }
            case ACE_Message_Block::MB_PCSIG:
                {
                    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Got MB_PCSIG \n")));
                    m_continue = false;
                    m_mb->release();
                    msg_queue()->deactivate();
                    break;
                }
            default:
                {
                    m_continue = false;
                    break;
                }
            }
        } else {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D [worker:%t] %M %N:%l Micro service is stopped\n")));
            m_continue = false;
        }
    }

    return(0);
}

MicroService::MicroService(ACE_Thread_Manager* thr_mgr) :
    ACE_Task<ACE_MT_SYNCH>(thr_mgr)
{
    m_continue = true;
    m_mb = nullptr;
    //ACE_NEW_NORETURN(m_mb, ACE_Message_Block((size_t)MemorySize::SIZE_1MB));
    m_threadId = thr_mgr->thr_self();
}

MicroService::~MicroService()
{
    m_mb->release();
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Microservice dtor is invoked\n")));
}

/*
 * _                  _   _ _ _    _          _ _ _ _  _ _ _ _ _ _ _ _ _   _        _  _ _ _ _ _ _ _ _ 
 * \\       /\       //  // _ \ \ ||         ||      || ||      ||      \\ \\      // ||      ||      \\
 *  \\     //\\     //_ //_ _ _\_\||_ _ _    || _  _    ||_ _   ||      //  \\    //  ||_ _   ||      //
 *   \\   //  \\   // -//         ||     \\          || ||      || _ //      \\  //   ||      || _ //
 *    \\_//    \\_//   \\_ _ _ _  ||_ _ _//  || _ _ _|| ||_ _ _ ||     \\     \\//    ||_ _ _ ||     \\
 *
 *                
 */


ACE_INT32 WebServer::handle_timeout(const ACE_Time_Value& tv, const void* act)
{

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l WebServer::handle_timedout\n")));
    std::uintptr_t handle = reinterpret_cast<std::uintptr_t>(act);
    auto conIt = m_connectionPool.find(handle);

    if(conIt != std::end(m_connectionPool)) {
        WebConnection* connEnt = conIt->second;
        /* let the reactor call handle_close on this handle */
        ACE_Reactor::instance()->remove_handler(handle, ACE_Event_Handler::READ_MASK);
        /* reclaim the heap memory */
        delete connEnt;
    }

    return(0);
}

ACE_INT32 WebServer::handle_input(ACE_HANDLE handle)
{
    int ret_status = 0;
    ACE_SOCK_Stream peerStream;
    ACE_INET_Addr peerAddr;
    WebConnection* connEnt = nullptr;

    ret_status = m_server.accept(peerStream, &peerAddr);

    if(!ret_status) {
        auto it = m_connectionPool.find(peerStream.get_handle());
        if(it != std::end(m_connectionPool)) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l Existing connection on handle %d found in connection pool\n"), peerStream.get_handle()));
            connEnt = it->second;
        } else {
            ACE_NEW_RETURN(connEnt, WebConnection(this), -1);
            m_connectionPool[peerStream.get_handle()] = connEnt;
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l New connection is created and handle is %d\n"), peerStream.get_handle()));
            /*! Start Handle Cleanup Timer to get rid of this handle from connectionPool*/
            long tId = start_conn_cleanup_timer(peerStream.get_handle());
            connEnt->timerId(tId);
            connEnt->handle(peerStream.get_handle());
            connEnt->connAddr(peerAddr);
            ACE_Reactor::instance()->register_handler(connEnt, 
                                                      ACE_Event_Handler::READ_MASK|ACE_Event_Handler::TIMER_MASK | ACE_Event_Handler::SIGNAL_MASK);
        }
    } else {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Master:%t] %M %N:%l Accept to new connection failed\n")));
    }

    return(0);
}

ACE_INT32 WebServer::handle_signal(int signum, siginfo_t* s, ucontext_t* ctx)
{
    ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Master:%t] %M %N:%l Signal Number %d and its name %S is received for WebServer\n"), signum, signum));
    if(!connectionPool().empty()) {
      for(auto it = connectionPool().begin(); it != connectionPool().end(); ++it) {
        auto wc = it->second;
        stop_conn_cleanup_timer(wc->timerId());
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Master:%t] %M %N:%l its name %S is received for WebServer\n"), signum));
        delete wc;
      }
      connectionPool().clear();
    }

    if(!workerPool().empty()) {
        std::for_each(workerPool().begin(), workerPool().end(), [&](MicroService* ms) {
            ACE_Message_Block* req = nullptr;
            ACE_NEW_RETURN(req, ACE_Message_Block((size_t)MemorySize::SIZE_1KB), -1);
            req->msg_type(ACE_Message_Block::MB_PCSIG);
            ms->putq(req);
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Master:%t] %M %N:%l Sending to Worker Node\n")));
        });
    }

    return(0);
}

ACE_INT32 WebServer::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
    return(0);
}

ACE_HANDLE WebServer::get_handle() const
{
    return(m_server.get_handle());
}

WebServer::WebServer(std::string ipStr, ACE_UINT16 listenPort, ACE_UINT32 workerPool,
                     std::string dbUri,
                     std::string dbConnPool,
                     std::string dbName)
{
    std::string addr;
    addr.clear();

    if(ipStr.length()) {
        addr = ipStr;
        addr += ":";
        addr += std::to_string(listenPort);
        m_listen.set_address(addr.c_str(), addr.length());
    } else {
        addr = std::to_string(listenPort);
        m_listen.set_port_number(listenPort);
    }

    /* Stop the Webserver when this m_stopMe becomes true. */
    m_stopMe = false;

    int reuse_addr = 1;
    if(m_server.open(m_listen, reuse_addr)) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l Starting of WebServer failed - opening of port %d hostname %s\n"), m_listen.get_port_number(), m_listen.get_host_name()));
    }

    m_workerPool.clear();
    std::uint32_t cnt;

    for(cnt = 0; cnt < workerPool; ++cnt) {
        MicroService* worker = nullptr;
        ACE_NEW_NORETURN(worker, MicroService(ACE_Thread_Manager::instance()));
        m_workerPool.push_back(worker);
        worker->open();
    }

    m_currentWorker = std::begin(m_workerPool);

    /* Mongo DB interface */
    std::string uri("mongodb://127.0.0.1:27017");
    std::string _dbName("bayt");
    std::uint32_t _pool = 50;

    if(dbUri.length()) {
        uri.assign(dbUri);
    }

    if(dbConnPool.length()) {
        _pool = std::stoi(dbConnPool);
    }

    if(dbName.length()) {
        _dbName.assign(dbName);
    }

    mMongodbc = new Mongodbc(uri, _dbName, _pool);
}

WebServer::~WebServer()
{
    if(nullptr != mMongodbc) {
        delete mMongodbc;
        mMongodbc = nullptr;
    }

    if(!workerPool().empty()) {
        for(auto it = workerPool().begin(); it != workerPool().end(); ++it) {
            auto ent = *it;
            delete ent;
        }
    }
}

bool WebServer::start()
{
    int ret_status = 0;
    ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::ACCEPT_MASK | ACE_Event_Handler::TIMER_MASK | ACE_Event_Handler::SIGNAL_MASK); 
    /* subscribe for signal */
    ACE_Sig_Set ss;
    ss.empty_set();
    ss.sig_add(SIGINT);
    ss.sig_add(SIGTERM);
    ACE_Reactor::instance()->register_handler(&ss, this); 

    ACE_Time_Value to(1,0);

    while(!m_stopMe) {
        ACE_Reactor::instance()->handle_events(to);
    }

    return(m_stopMe);
}

bool WebServer::stop()
{
    return(true);
}

long WebServer::start_conn_cleanup_timer(ACE_HANDLE handle)
{
    long timerId = -1;
    ACE_Time_Value to(20,0);
    timerId = ACE_Reactor::instance()->schedule_timer(this, (const void *)handle, to);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l webserver cleanup timer is started for handle %d\n"), handle));
    return(timerId);
}

void WebServer::stop_conn_cleanup_timer(long timerId) 
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l webserver connection cleanup timer is stopped\n")));
    ACE_Reactor::instance()->cancel_timer(timerId);
}
void WebServer::restart_conn_cleanup_timer(ACE_HANDLE handle)
{
    ACE_Time_Value to(20,0);
    auto conIt = connectionPool().find(handle);

    if(conIt != std::end(connectionPool())) {
        auto connEnt = conIt->second;
        long tId = connEnt->timerId();
        ACE_Reactor::instance()->reset_timer_interval(tId, to);
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l webserver connection cleanup timer is re-started for handle %d\n"), handle));
    }

}

WebConnection::WebConnection(WebServer* parent)
{
    m_timerId = -1;
    m_handle = -1;
    m_parent = parent;
}

WebConnection::~WebConnection()
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l WebConnection dtor is invoked\n")));
}

ACE_INT32 WebConnection::handle_timeout(const ACE_Time_Value &tv, const void *act)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l Webconnection::handle_timedout\n")));
    std::uintptr_t handle = reinterpret_cast<std::uintptr_t>(act);
    auto conIt = m_parent->connectionPool().find(handle);

    if(conIt != std::end(m_parent->connectionPool())) {
        auto connEnt = conIt->second;
        m_parent->connectionPool().erase(conIt);
    }
    return(0);
}

ACE_INT32 WebConnection::handle_input(ACE_HANDLE handle)
{
    ACE_Message_Block* req;
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l WebConnection::handle_input\n")));

    ACE_NEW_NORETURN(req, ACE_Message_Block((size_t)MemorySize::SIZE_1MB));
    req->msg_type(ACE_Message_Block::MB_DATA);
    /*_ _ _ _ _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
     | 4-bytes handle   | 4-bytes db instance pointer   | request (payload) |
     |_ _ _ _ _ _ _ _ _ |_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _|_ _ _ _ _ _ _ _ _ _|
     */
    *((ACE_HANDLE *)req->wr_ptr()) = handle;
    req->wr_ptr(sizeof(ACE_HANDLE));

    /* db instance */
    std::uintptr_t inst = reinterpret_cast<std::uintptr_t>(parent()->mongodbcInst());
    *((std::uintptr_t* )req->wr_ptr()) = inst;

    req->wr_ptr(sizeof(uintptr_t));

    std::int32_t len = recv(handle, req->wr_ptr(), (size_t)MemorySize::SIZE_1MB, 0);
    if(len < 0) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("%D %M %t:%N:%l Receive failed for handle %d\n"), handle));
        return(len);
    }

    if(!len) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l Request of length %d on connection %d\n"), len, handle));
        return(-1);
    }

    req->wr_ptr(len);

    auto it = m_parent->currentWorker();
    MicroService* mEnt = *it;
    mEnt->putq(req);
    /* re-start the connection timed out timer again */
    parent()->restart_conn_cleanup_timer(handle);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l Connection timeout timer is re-started for handle %d\n"), handle));
    return(0);
}

ACE_INT32 WebConnection::handle_signal(int signum, siginfo_t *s, ucontext_t *u)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l signal number - %d (%S) is received\n"), signum));
    if(m_timerId > 0) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l Running timer is stopped for signal (%S)\n"), signum));
        m_parent->stop_conn_cleanup_timer(m_timerId);
    }
    return(0);
}

ACE_INT32 WebConnection::handle_close (ACE_HANDLE handle, ACE_Reactor_Mask mask)
{
    if(m_timerId > 0) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l Running timer for handle %d is stopped\n"), handle));
        m_parent->stop_conn_cleanup_timer(m_timerId);
    }

    auto it = m_parent->connectionPool().find(handle);
    if(it != std::end(m_parent->connectionPool())) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l Entry is removed from connection pool\n")));
        it = m_parent->connectionPool().erase(it);
        close(handle);
    }

    return(0);
}

ACE_HANDLE WebConnection::get_handle() const
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D %M %t:%N:%l WebConnection::get_handle - handle %d\n"), m_handle));
    return(m_handle);
}


#endif /* __webservice_cc__*/
