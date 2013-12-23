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

#ifndef DB_TWILIO_HH
#define DB_TWILIO_HH

#include <stdexcept>
#include <string>
#include <memory>
#include <map>

#include "curl.hh"
#include "jsonxx.hh"
#include "xmlxx.hh"

namespace databracket
{

namespace twilio
{

namespace http
{

    enum methods {
        GET,
        POST,
        PUT,
        DELETE
    };

    enum status {
        OK_200 = 200, // SUCCESS, body contains representation requested.
        OK_201 = 201, // SUCCESS, updated resource, body contains resource.
        OK_204 = 204, // SUCCESS, deleted resource, body is empty.
        // Hopefully we will never encounter that code as curl is instructed
        // to follow all redirects.
        FOUND_302 = 302, // REDIRECT.
        NOT_MODIFIED_304 = 304, // Client's cache version is up-to-date.
        BAD_REQUEST_400 = 400, // POST parameters are invalid.
        UNAUTHORIZED_401 = 401,
        NOT_FOUND_404 = 404, // You know this one.
        SERVER_ERROR_500 = 500 // Twilio just farted, please try again.
    };

} // namespace http

    using std::map;
    using std::string;
    using std::auto_ptr;
    using std::runtime_error;

    // Forward declaration for ::response.
    struct response;

    // restclient

    class restclient {
    public:
        restclient(const string& sid, const string& autht,
                const string& endpoint = "https://api.twilio.com");
        ~restclient();

        response request(const string& path, http::methods method,
                const map<string, string>& vars, bool json = true);
        response send_sms(const string& to, const string& from,
                const string& body, const string& statuscallback,
                bool json = true);
        response send_sms(const string& to, const string& from,
                const string& body, bool json = true);

    private:
        string urlify(CURL* cinst, const map<string, string>& vars) const;
        static size_t write_data(void* ptr, size_t size, size_t nmemb,
                void* userdata);
        static size_t read_data(void* ptr, size_t size, size_t nmemb,
                void* userdata);

        friend class response;

    private:
        string sid_;
        string autht_;
        string endpoint_;
    };

    // response

    struct response {
        response(const response& ref);

        http::status http_status() const;
        const string& result() const;
        bool iserror() const;
        string message() const;
        unsigned int code() const;

    private:
        typedef auto_ptr<jsonxx::Object> obj_ptr;
        response(const string& result, http::status rescode, bool json);

        friend class restclient;

    private:
        XMLNode xnod_;
        obj_ptr obj_;
        http::status status_;
        string result_;
        bool json_;
    };

} // namespace twilio

} // namespace databracket

#endif // DB_TWILIO_HH
