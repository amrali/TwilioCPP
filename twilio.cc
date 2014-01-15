/*  Copyright (c) 2011 Databracket LLC
 *  All rights reserved.
 *
 *  Author: Amr Ali <amr@databracket.com>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Databracket LLC nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <cstring>
#include <sstream>

#include "singleton.hh"
#include "twilio.hh"

namespace databracket
{

namespace twilio
{

    // restclient

    restclient::restclient(const string& sid, const string& autht,
            const string& endpoint) :
        sid_(sid), autht_(autht), endpoint_(endpoint)
    {}

    restclient::~restclient()
    {
        curl_global_cleanup();
    }

    response
    restclient::request(const string& path, http::methods method,
            const map<string, string>& vars, bool json)
    {
        CURL* c = curl_easy_init();

        if (!c)
            throw curl::curl_error("something went wrong initializing curl");

        string result;
        string url = endpoint_ + "/" + path;
        string userpwd = sid_ + ":" + autht_;
        string data;

        if (json) {
            url += ".json";
        } else {
            url += ".xml";
        }

        if (!vars.empty())
            data = urlify(c, vars);

        if (!data.empty() && method == http::GET)
            url += "?" + data;

        struct curl_slist* headers = NULL;

        headers = curl_slist_append(headers,
                "User-Agent: Databracket Twilio Client v1.0");

        if (!headers) {
            curl_easy_cleanup(c);
            throw std::bad_alloc();
        }

        curl_easy_setopt(c, CURLOPT_URL, url.c_str());
        curl_easy_setopt(c, CURLOPT_SHARE,
                singleton<curl::share>::instance()->get());
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(c, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(c, CURLOPT_USERPWD, userpwd.c_str());
        curl_easy_setopt(c, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION,
                &(restclient::write_data));
        curl_easy_setopt(c, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(c, CURLOPT_NOPROGRESS, true);
        curl_easy_setopt(c, CURLOPT_TIMEOUT, 10);
        curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1);

        switch (method) {
            case http::GET:
                curl_easy_setopt(c, CURLOPT_HTTPGET, true);
                break;

            case http::POST:
                curl_easy_setopt(c, CURLOPT_POST, true);
                curl_easy_setopt(c, CURLOPT_POSTFIELDS, data.c_str());
                break;

            case http::PUT:
                curl_easy_setopt(c, CURLOPT_UPLOAD, true);
                curl_easy_setopt(c, CURLOPT_READFUNCTION,
                        &(restclient::read_data));
                curl_easy_setopt(c, CURLOPT_READDATA, &data);
                curl_easy_setopt(c, CURLOPT_INFILESIZE_LARGE,
                        (curl_off_t)data.size());
                break;

            case http::DELETE:
                curl_easy_setopt(c, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;

            default:
                curl_slist_free_all(headers);
                curl_easy_cleanup(c);
                throw runtime_error("unsupported http method");
                break;
        }

        CURLcode cc = curl_easy_perform(c);

        if (cc) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(c);
            throw curl::curl_error(curl_easy_strerror(cc));
        }

        curl_slist_free_all(headers);

        unsigned long rescode;

        cc = curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &rescode);

        if (cc) {
            curl_easy_cleanup(c);
            throw curl::curl_error(curl_easy_strerror(cc));
        }

        curl_easy_cleanup(c);

        return response(result, (http::status)rescode, json);
    }

    response
    restclient::send_sms(const string& to, const string& from,
            const string& body, const string& statuscallback, bool json)
    {
        if (body.size() > 1600) // see Twilio specifications
            throw runtime_error("sms body can only hold 1600 characters max");

        string path = "2010-04-01/Accounts/" + sid_ + "/Messages";

        map<string, string> vars;

        vars["To"] = to;
        vars["From"] = from;
        vars["Body"] = body;

        if (!statuscallback.empty())
            vars["StatusCallback"] = statuscallback;

        return request(path, http::POST, vars, json);
    }

    response
    restclient::send_sms(const string& to, const string& from,
            const string& body, bool json)
    {
        return send_sms(to, from, body, "", json);
    }

    string
    restclient::urlify(CURL* cinst, const map<string, string>& vars) const
    {
        std::ostringstream out;
        map<string, string>::const_iterator it;

        for (it = vars.begin(); it != vars.end(); ++it) {
            if (it != vars.begin())
                out << '&';

            char* second = curl_easy_escape(cinst,
                    it->second.c_str(), it->second.size());

            out << it->first << '=' << second;

            curl_free(second);
        }

        return out.str();
    }

    size_t
    restclient::write_data(void* ptr, size_t size, size_t nmemb,
            void* userdata)
    {
        size_t total = size * nmemb;

        (*(string*)userdata).append((const char*)ptr, total);

        return total;
    }

    size_t
    restclient::read_data(void* ptr, size_t size, size_t nmemb,
            void* userdata)
    {
        size_t max = size * nmemb;

        string data = (*(string*)userdata).substr(0, max);
        (*(string*)userdata).erase(0, data.size());

        memcpy(ptr, data.c_str(), data.size());
        return data.size();
    }

    // response

    response::response(const string& result, http::status rescode, bool json) :
        status_(rescode), result_(result), json_(json)
    {
        if (status_ == http::OK_204)
            return;

        if (json_) {
            std::istringstream iss(result_);
            obj_ = obj_ptr(new jsonxx::Object());

            if (!jsonxx::Object::parse(iss, *(obj_)))
                throw runtime_error("syntax error in Twilio JSON response");

        } else {
            xnod_ = XMLNode::parseString(result_.c_str());

            if (xnod_.isEmpty())
                throw runtime_error("syntax error in Twilio XML response");

        }
    }

    response::response(const response& ref) :
        xnod_(ref.xnod_), obj_(obj_ptr(ref.obj_.get())), status_(ref.status_),
        result_(ref.result_), json_(ref.json_)
    {}

    http::status
    response::http_status() const
    {
        return status_;
    }

    const string&
    response::result() const
    {
        return result_;
    }

    bool
    response::iserror() const
    {
        return status_ >= http::BAD_REQUEST_400;
    }

    string
    response::message() const
    {
        if (json_) {
            if (obj_->has<string>("message"))
                return obj_->get<string>("message");

        } else {
            const XMLNode& restex = xnod_.getChildNode("TwilioResponse")
                .getChildNode("RestException").getChildNode("Message");

            if (restex.nText())
                return restex.getText();
        }

        return "";
    }

    unsigned int
    response::code() const
    {
        if (json_) {
            if (obj_->has<double>("code"))
                return obj_->get<double>("code");

        } else {
            const XMLNode& restex = xnod_.getChildNode("TwilioResponse")
                .getChildNode("RestException").getChildNode("Code");

            if (restex.nText()) {
                unsigned int code;
                std::istringstream iss(restex.getText());

                iss >> code;

                return code;
            }
        }

        return 0;
    }

} // namespace twilio

} // namespace databracket
