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

#include <pthread.h>

#include "curl.hh"

namespace databracket
{

namespace curl
{

	pthread_mutex_t share_mutex = PTHREAD_MUTEX_INITIALIZER;

	share::share()
	{
		CURLcode cc = curl_global_init(CURL_GLOBAL_SSL);

		if (cc)
			throw curl_error(curl_easy_strerror(cc));

		csh_ = curl_share_init();

		if (!csh_) {
			curl_share_cleanup(csh_);
			throw curl_error("something went wrong initializing curl share");
		}

		CURLSHcode csc;

		if ((csc = curl_share_setopt(csh_, CURLSHOPT_LOCKFUNC,
						&(share::lock_function)))) {
			curl_share_cleanup(csh_);
			throw curl_error(curl_share_strerror(csc));
		}

		if ((csc = curl_share_setopt(csh_, CURLSHOPT_UNLOCKFUNC,
					&(share::unlock_function)))) {
			curl_share_cleanup(csh_);
			throw curl_error(curl_share_strerror(csc));
		}

		if ((csc = curl_share_setopt(csh_, CURLSHOPT_SHARE,
					CURL_LOCK_DATA_DNS))) {
			curl_share_cleanup(csh_);
			throw curl_error(curl_share_strerror(csc));
		}
	}

	share::~share()
	{
		curl_share_cleanup(csh_);
	}

	CURLSH*
	share::get() throw()
	{
		return csh_;
	}

	void
	share::lock_function(CURLSH*, curl_lock_data, curl_lock_access, void*)
	{
		pthread_mutex_lock(&share_mutex);
	}

	void
	share::unlock_function(CURLSH*, curl_lock_data, void*)
	{
		pthread_mutex_unlock(&share_mutex);
	}

} // namespace curl

} // namespace databracket
