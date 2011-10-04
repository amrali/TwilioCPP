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

#ifndef __DB_CURL_HH
#define __DB_CURL_HH

#include <stdexcept>
#include <string>

#include <curl/curl.h>

namespace databracket
{

namespace curl
{

    using std::string;
    using std::runtime_error;

    /*
     * curl_error: A class to distinguish curl related exceptions from the rest.
     */

    class curl_error : public runtime_error {
    public:
        curl_error(const string& err) : runtime_error(err) {}
        ~curl_error() throw() {}
    };

    /*
     * share: A structure to facilitate a global usage of CURLSH.
     */
    struct share {
        share();
        ~share();

        CURLSH* get() throw();

    private:
        static void lock_function(CURL*, curl_lock_data,
                curl_lock_access, void*);
        static void unlock_function(CURL*, curl_lock_data,
                void*);

    private:
        CURLSH* csh_;
    };

} // namespace curl

} // namespace databracket

#endif // __DB_CURL_HH
